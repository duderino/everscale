#ifndef ESB_CONSOLE_LOGGER_H
#include <ESBConsoleLogger.h>
#endif

#ifndef ES_HTTP_STACK_H
#include <ESHttpStack.h>
#endif

#ifndef ES_HTTP_ECHO_SERVER_HANDLER_H
#include <ESHttpEchoServerHandler.h>
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

#ifndef ES_HTTP_CLIENT_SIMPLE_COUNTERS_H
#include <ESHttpClientSimpleCounters.h>
#endif

#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

#ifndef ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <ESHttpEchoClientRequestBuilder.h>
#endif

#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
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

int main(int argc, char **argv) {
  int clientThreads = 3;
  int serverThreads = 3;
  const char *host = "localhost.localdomain";
  int port = 8888;
  unsigned int connections = 100;  // concurrent connections
  unsigned int iterations = 100;   // http requests per concurrent connection
  bool reuseConnections = true;
  int logLevel = ESB::Logger::Notice;
  const char *method = "GET";
  const char *contentType = "octet-stream";
  const char *absPath = "/";
  FILE *outputFile = stdout;
  const time_t windowSizeSec = 1;
  unsigned char body[1024];

  memset(body, 42, sizeof(body));

  {
    int result = 0;

    while (true) {
      result = getopt(argc, argv, "l:t:c:i:r:");

      if (0 > result) {
        break;
      }

      switch (result) {
        case 'l':

          logLevel = atoi(optarg);
          break;

        case 't':

          clientThreads = atoi(optarg);
          serverThreads = clientThreads;
          break;

        case 'c':

          connections = (unsigned int) atoi(optarg);
          break;

        case 'i':

          iterations = (unsigned int) atoi(optarg);
          break;

        case 'r':

          reuseConnections = 0 != atoi(optarg);
          break;
      }
    }
  }

  ESB::ConsoleLogger::Initialize((ESB::Logger::Severity)logLevel);
  ESB::Logger *logger = ESB::ConsoleLogger::Instance();

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  // Init

  HttpEchoServerHandler serverHandler(logger);
  ESB::SystemDnsClient dnsClient(logger);
  HttpClientSimpleCounters clientCounters;
  HttpServerSimpleCounters serverCounters;

  HttpStack serverStack(&serverHandler, &dnsClient, port, serverThreads,
                        &clientCounters, &serverCounters, logger);

  ESB::Error error = serverStack.initialize();

  if (ESB_SUCCESS != error) {
    return -1;
  }

  HttpClientHistoricalCounters counters(
      windowSizeSec, ESB::SystemAllocator::GetInstance(), logger);
  HttpStack clientStack(&dnsClient, clientThreads, &counters, logger);
  HttpEchoClientHandler clientHandler(absPath, method, contentType, body,
                                      sizeof(body), connections * iterations,
                                      &clientStack, logger);
  HttpClientSocket::SetReuseConnections(reuseConnections);

  error = clientStack.initialize();

  if (ESB_SUCCESS != error) {
    return -2;
  }

  // Start

  error = serverStack.start();

  if (ESB_SUCCESS != error) {
    return -3;
  }

  error = clientStack.start();

  if (ESB_SUCCESS != error) {
    return -4;
  }

  sleep(1);  // give the worker threads a chance to start

  // Send traffic

  ESB::DiscardAllocator allocator(4096, ESB::SystemAllocator::GetInstance());
  HttpEchoClientContext *context = 0;
  HttpClientTransaction *transaction = 0;

  for (int i = 0; i < connections; ++i) {
    // Create the request context and transaction
    context = new (&allocator) HttpEchoClientContext(iterations - 1);
    assert(context);

    transaction = clientStack.createClientTransaction(&clientHandler);
    assert(transaction);

    transaction->setApplicationContext(context);

    // Build the request

    error = HttpEchoClientRequestBuilder(host, port, absPath, method,
                                         contentType, transaction);
    assert(ESB_SUCCESS == error);

    // Send the request (asynch) - the context will resubmit the request for
    // <iteration> - 1 iterations.

    error = clientStack.executeClientTransaction(transaction);
    assert(ESB_SUCCESS == error);
  }

  while (!clientHandler.isFinished()) {
    sleep(1);
  }

  // Stop

  error = clientStack.stop();
  assert(ESB_SUCCESS == error);
  error = serverStack.stop();
  assert(ESB_SUCCESS == error);

  // TODO assert on counters here
  clientStack.getClientCounters()->printSummary(outputFile);
  serverCounters.printSummary(outputFile);
  fflush(outputFile);
  fclose(outputFile);

  // Destroy

  clientStack.destroy();
  serverStack.destroy();
  allocator.destroy();  // context destructors will not be called.

  ESB::ConsoleLogger::Destroy();

  return ESB_SUCCESS;
}
