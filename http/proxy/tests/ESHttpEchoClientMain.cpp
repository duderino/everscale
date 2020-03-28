#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_STACK_H
#include <ESHttpStack.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#include <ESHttpEchoClientContext.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_HANDLER_H
#include <ESHttpEchoClientHandler.h>
#endif

#ifndef ESB_SYSTEM_DNS_CLIENT_H
#include <ESBSystemDnsClient.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <ESHttpEchoClientRequestBuilder.h>
#endif

#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace ES;

static void RawEchoClientSignalHandler(int signal);
static volatile ESB::Word IsRunning = 1;

static void printHelp(const char *progName) {
  fprintf(stderr, "Usage: %s <options>\n", progName);
  fprintf(stderr, "\n");
  fprintf(stderr, "\tOptions:\n");
  fprintf(stderr,
          "\t-l <logLevel>     Defaults to 7 / INFO.  See below for other "
          "levels.\n");
  fprintf(
      stderr,
      "\t-t <epollThreads> Defaults to 1 thread handing all connections.\n");
  fprintf(stderr, "\t-H <serverHost>   Defaults to localhost\n");
  fprintf(stderr, "\t-p <serverPort>   Defaults to 8080\n");
  fprintf(stderr, "\t-c <connections>  Defaults to 1 connection\n");
  fprintf(stderr, "\t-i <iterations>   Defaults to 1 request per connection\n");
  fprintf(stderr, "\t-m <method>       Defaults to GET\n");
  fprintf(stderr,
          "\t-C <content-type> Defaults to octet-stream if there is a "
          "non-empty body\n");
  fprintf(
      stderr,
      "\t                  Otherwise no Content-Type header will be sent\n");
  fprintf(stderr, "\t-b <body file>    Defaults to an empty body\n");
  fprintf(stderr, "\t-a <abs_path>     Defaults to '/'\n");
  fprintf(stderr,
          "\t-o <output file>  Defaults to stdout.  Perf results will be "
          "printed here\n");
  fprintf(stderr,
          "\t-r                Enables persistent connections, persistent "
          "connections\n");
  fprintf(stderr, "\t                  will not be used if not specified\n");
  fprintf(stderr, "\n");
  fprintf(stderr,
          "\tLog Levels:\n"
          "\tNone = 0,         All logging disabled.\n"
          "\tEmergency = 1,    System-wide non-recoverable error.\n"
          "\tAlert = 2,        System-wide non-recoverable error imminent.\n"
          "\tCritical = 3,     System-wide potentially recoverable error.\n"
          "\tError = 4,        Localized non-recoverable error.\n"
          "\tWarning = 5,      Localized potentially recoverable error.\n"
          "\tNotice = 6,       Important non-error event.\n"
          "\tInfo = 7,         Non-error event.\n"
          "\tDebug = 8         Debugging event.\n");
}

int main(int argc, char **argv) {
  int threads = 1;
  const char *host = "localhost.localdomain";
  int port = 8080;
  int connections = 1;
  int iterations = 1;
  bool reuseConnections = false;
  int logLevel = ESB::Logger::Notice;
  const char *method = "GET";
  const char *contentType = "octet-stream";
  const char *bodyFilePath = 0;
  const char *absPath = "/";

  {
    int result = 0;

    while (true) {
      result = getopt(argc, argv, "l:t:H:p:c:i:m:C:b:a:rh");

      if (0 > result) {
        break;
      }

      switch (result) {
        case 'l':

          logLevel = atoi(optarg);
          break;

        case 't':

          threads = atoi(optarg);
          break;

        case 'H':

          host = optarg;
          break;

        case 'p':

          port = atoi(optarg);
          break;

        case 'c':

          connections = atoi(optarg);
          break;

        case 'i':

          iterations = atoi(optarg);
          break;

        case 'm':

          method = optarg;
          break;

        case 'C':

          contentType = optarg;
          break;

        case 'b':

          bodyFilePath = optarg;
          break;

        case 'a':

          absPath = optarg;
          break;

        case 'r':

          reuseConnections = true;
          break;

        case 'h':

          printHelp(argv[0]);
          return 0;

        default:

          printHelp(argv[0]);

          return 2;
      }
    }
  }

  ESB::SimpleFileLogger logger;
  logger.setSeverity((ESB::Logger::Severity)logLevel);
  ESB::Logger::SetInstance(&logger);

  ESB_LOG_NOTICE(
      "Starting. logLevel: %d, threads: %d, host: %s, port: %d, "
      "connections: %d, iterations: %d, method: %s, contentType: %s, "
      "bodyFile %s, reuseConnections: %s",
      logLevel, threads, host, port, connections, iterations, method,
      contentType, bodyFilePath, reuseConnections ? "true" : "false");

  //
  // Install signal handlers: Ctrl-C and kill will start clean shutdown sequence
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, RawEchoClientSignalHandler);
  signal(SIGQUIT, RawEchoClientSignalHandler);
  signal(SIGTERM, RawEchoClientSignalHandler);

  //
  // Max out open files
  //

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(
      ESB::SystemConfig::Instance().socketHardMax());

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot raise max fd limit");
    return -5;
  }

  //
  // Slurp the request body into memory
  //

  int bodySize = 0;
  unsigned char *body = 0;

  if (0 != bodyFilePath) {
    int fd = open(bodyFilePath, O_RDONLY);
    if (-1 == fd) {
      fprintf(stderr, "Cannot open %s: %s\n", bodyFilePath, strerror(errno));
      return -5;
    }

    struct stat statbuf;

    memset(&statbuf, 0, sizeof(statbuf));

    if (0 != fstat(fd, &statbuf)) {
      close(fd);
      fprintf(stderr, "Cannot stat %s: %s\n", bodyFilePath, strerror(errno));
      return -6;
    }

    bodySize = statbuf.st_size;

    if (0 >= bodySize) {
      close(fd);
      bodySize = 0;
      bodyFilePath = 0;
    } else {
      body = (unsigned char *)malloc(bodySize);

      if (0 == body) {
        close(fd);
        fprintf(stderr, "Cannot allocate %d bytes of memory for body file\n",
                bodySize);
        return -7;
      }

      int bytesRead = 0;
      int totalBytesRead = 0;

      while (totalBytesRead < bodySize) {
        bytesRead = read(fd, body + totalBytesRead, bodySize - totalBytesRead);

        if (0 == bytesRead) {
          free(body);
          close(fd);
          fprintf(stderr, "Premature EOF slurping body into memory\n");
          return -8;
        }

        if (0 > bytesRead) {
          free(body);
          close(fd);
          fprintf(stderr, "Error slurping %s into memory: %s\n", bodyFilePath,
                  strerror(errno));
          return -9;
        }

        totalBytesRead += bytesRead;
      }

      close(fd);
    }
  }

  //
  // Create, initialize, and start the stack
  //

  ESB::SystemDnsClient dnsClient;
  HttpClientHistoricalCounters counters(1000, 30,
                                        ESB::SystemAllocator::Instance());
  HttpStack stack(&dnsClient, threads, &counters);
  HttpEchoClientHandler handler(absPath, method, contentType, body, bodySize,
                                connections * iterations, &stack);

  // TODO - make configuration stack-specific and increase options richness
  HttpClientSocket::SetReuseConnections(reuseConnections);

  error = stack.initialize();

  if (ESB_SUCCESS != error) {
    if (body) {
      free(body);
      body = 0;
    }

    return -1;
  }

  error = stack.start();

  if (ESB_SUCCESS != error) {
    if (body) {
      free(body);
      body = 0;
    }

    return -2;
  }

  ESB::DiscardAllocator echoClientContextAllocator(
      1024, 64, ESB::SystemAllocator::Instance());

  sleep(1);  // give the worker threads a chance to start - cleans up perf
             // testing numbers a bit

  // Create <connections> distinct client connections which each submit
  // <iterations> SOAP requests

  HttpEchoClientContext *context = 0;
  HttpClientTransaction *transaction = 0;

  for (int i = 0; i < connections; ++i) {
    // Create the request context and transaction

    context =
        new (&echoClientContextAllocator) HttpEchoClientContext(iterations - 1);

    if (0 == context) {
      ESB_LOG_ERROR("[main] cannot create new client context");

      if (body) {
        free(body);
        body = 0;
      }

      return -3;
    }

    transaction = stack.createClientTransaction(&handler);

    if (0 == transaction) {
      context->~HttpEchoClientContext();
      echoClientContextAllocator.deallocate(context);

      ESB_LOG_ERROR("[main] cannot create new client transaction");

      if (body) {
        free(body);
        body = 0;
      }

      return -3;
    }

    transaction->setApplicationContext(context);

    // Build the request

    error = HttpEchoClientRequestBuilder(host, port, absPath, method,
                                         contentType, transaction);

    if (ESB_SUCCESS != error) {
      context->~HttpEchoClientContext();
      echoClientContextAllocator.deallocate(context);
      stack.destroyClientTransaction(transaction);

      ESB_LOG_ERROR_ERRNO(error, "[main] cannot build request");

      if (body) {
        free(body);
        body = 0;
      }

      return -4;
    }

    // Send the request (asynch) - the context will resubmit the request for
    // <iteration> - 1 iterations.

    error = stack.executeClientTransaction(transaction);

    if (ESB_SUCCESS != error) {
      context->~HttpEchoClientContext();
      echoClientContextAllocator.deallocate(context);
      stack.destroyClientTransaction(transaction);

      ESB_LOG_ERROR_ERRNO(error, "[main] Cannot execute client transaction");

      if (body) {
        free(body);
        body = 0;
      }

      return error;
    }
  }

  while (IsRunning && false == handler.isFinished()) {
    sleep(5);
  }

  error = stack.stop();

  if (body) {
    free(body);
    body = 0;
  }

  if (ESB_SUCCESS != error) {
    return -3;
  }

  stack.getClientCounters()->log(ESB::Logger::Instance(),
                                 ESB::Logger::Severity::Notice);
  stack.destroy();

  return ESB_SUCCESS;
}

void RawEchoClientSignalHandler(int signal) { IsRunning = 0; }
