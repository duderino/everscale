#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#include <ESHttpLoadgenSeedCommand.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CLIENT_H
#include <ESHttpClient.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

static volatile ESB::Word IsRunning = 1;
static void SignalHandler(int signal) { IsRunning = 0; }

using namespace ES;

static void printHelp(const char *progName) {
  fprintf(stderr, "Usage: %s <options>\n", progName);
  fprintf(stderr, "\n");
  fprintf(stderr, "\tOptions:\n");
  fprintf(stderr,
          "\t-l <logLevel>     Defaults to 7 / INFO.  See below for other "
          "levels.\n");
  fprintf(stderr, "\t-t <epollThreads> Defaults to 1 thread handing all connections.\n");
  fprintf(stderr, "\t-H <serverHost>   Defaults to localhost\n");
  fprintf(stderr, "\t-p <serverPort>   Defaults to 8080\n");
  fprintf(stderr, "\t-c <connections>  Defaults to 1 connection\n");
  fprintf(stderr, "\t-i <iterations>   Defaults to 1 request per connection\n");
  fprintf(stderr, "\t-m <method>       Defaults to GET\n");
  fprintf(stderr,
          "\t-C <content-type> Defaults to octet-stream if there is a "
          "non-empty body\n");
  fprintf(stderr, "\t                  Otherwise no Content-Type header will be sent\n");
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
  ESB::SocketAddress destaddr("127.0.0.1", port, ESB::SocketAddress::TransportType::TCP);

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

  HttpLoadgenContext::SetTotalIterations(connections * iterations);

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity((ESB::Logger::Severity)logLevel);
  ESB::Logger::SetInstance(&logger);

  ESB_LOG_NOTICE(
      "[main] Starting. logLevel: %d, threads: %d, host: %s, port: %d, "
      "connections: %d, iterations: %d, method: %s, contentType: %s, "
      "bodyFile %s, reuseConnections: %s",
      logLevel, threads, host, port, connections, iterations, method, contentType, bodyFilePath,
      reuseConnections ? "true" : "false");

  //
  // Install signal handlers: Ctrl-C and kill will start clean shutdown sequence
  //

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, SignalHandler);
  signal(SIGQUIT, SignalHandler);
  signal(SIGTERM, SignalHandler);

  //
  // Max out open files
  //

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(ESB::SystemConfig::Instance().socketHardMax());

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[main] cannot raise max fd limit");
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
      fprintf(stderr, "[main] cannot open %s: %s\n", bodyFilePath, strerror(errno));
      return -5;
    }

    struct stat statbuf;

    memset(&statbuf, 0, sizeof(statbuf));

    if (0 != fstat(fd, &statbuf)) {
      close(fd);
      fprintf(stderr, "[main] cannot stat %s: %s\n", bodyFilePath, strerror(errno));
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
        fprintf(stderr, "[main] cannot allocate %d bytes of memory for body file\n", bodySize);
        return -7;
      }

      int bytesRead = 0;
      int totalBytesRead = 0;

      while (totalBytesRead < bodySize) {
        bytesRead = read(fd, body + totalBytesRead, bodySize - totalBytesRead);

        if (0 == bytesRead) {
          free(body);
          close(fd);
          fprintf(stderr, "[main] premature EOF slurping body into memory\n");
          return -8;
        }

        if (0 > bytesRead) {
          free(body);
          close(fd);
          fprintf(stderr, "[main] error slurping %s into memory: %s\n", bodyFilePath, strerror(errno));
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

  HttpLoadgenHandler clientHandler(absPath, method, contentType, body, sizeof(body), -1);
  HttpClientSocket::SetReuseConnections(reuseConnections);
  HttpClient client("loadgen", threads, clientHandler);

  error = client.initialize();

  if (ESB_SUCCESS != error) {
    if (body) {
      free(body);
      body = 0;
    }

    return -1;
  }

  error = client.start();

  if (ESB_SUCCESS != error) {
    if (body) {
      free(body);
      body = 0;
    }

    return -2;
  }

  for (int i = 0; i < client.threads(); ++i) {
    HttpLoadgenSeedCommand *command = new (ESB::SystemAllocator::Instance())
        HttpLoadgenSeedCommand(connections / threads, iterations, destaddr, port, host, absPath, method, contentType,
                               ESB::SystemAllocator::Instance().cleanupHandler());
    error = client.push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "[main] cannot push seed command");
      return -6;
    }
  }

  while (IsRunning && !HttpLoadgenContext::IsFinished()) {
    sleep(1);
  }

  error = client.stop();

  if (body) {
    free(body);
    body = 0;
  }

  if (ESB_SUCCESS != error) {
    return -4;
  }

  client.clientCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  client.destroy();

  return ESB_SUCCESS;
}
