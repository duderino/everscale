#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ES_HTTP_CLIENT_H
#include <ESHttpClient.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
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

#ifndef ES_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#include <ESHttpEchoClientRequestBuilder.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

using namespace ES;

int main(int argc, char **argv) {
  int clientThreads = 3;
  int serverThreads = 3;
  const char *host = "localhost.localdomain";
  int port = 8888;
  unsigned int connections = 500;  // concurrent connections
  unsigned int iterations = 500;   // http requests per concurrent connection
  bool reuseConnections = true;
  int logLevel = ESB::Logger::Notice;
  const char *method = "GET";
  const char *contentType = "octet-stream";
  const char *absPath = "/";
  const ESB::UInt16 maxWindows = 1000;
  const ESB::UInt16 windowSizeSec = 1;
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

          connections = (unsigned int)atoi(optarg);
          break;

        case 'i':

          iterations = (unsigned int)atoi(optarg);
          break;

        case 'r':

          reuseConnections = 0 != atoi(optarg);
          break;
      }
    }
  }

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity((ESB::Logger::Severity)logLevel);
  ESB::Logger::SetInstance(&logger);

  signal(SIGHUP, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  //
  // Max out open files
  //

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(
      ESB::SystemConfig::Instance().socketHardMax());

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot raise max fd limit");
    return -5;
  }

  // Init

  HttpEchoServerHandler serverHandler;
  HttpServer server(serverThreads, port, serverHandler);

  error = server.initialize();

  if (ESB_SUCCESS != error) {
    return -1;
  }

  HttpEchoClientHandler clientHandler(absPath, method, contentType, body,
                                      sizeof(body), connections * iterations);
  HttpClientSocket::SetReuseConnections(reuseConnections);
  HttpClient client(clientThreads, connections, TODO, clientHandler);

  error = client.initialize();

  if (ESB_SUCCESS != error) {
    return -2;
  }

  // Start

  error = server.start();

  if (ESB_SUCCESS != error) {
    return -3;
  }

  error = client.start();

  if (ESB_SUCCESS != error) {
    return -4;
  }

  // Send traffic

  ESB::DiscardAllocator allocator(ESB::SystemConfig::Instance().pageSize(),
                                  ESB::SystemConfig::Instance().cacheLineSize(),
                                  ESB::SystemAllocator::Instance());
  HttpEchoClientContext *context = 0;
  HttpClientTransaction *transaction = 0;

  for (unsigned int i = 0; i < connections; ++i) {
    // Create the request context and transaction
    context = new (&allocator) HttpEchoClientContext(iterations - 1);
    assert(context);

    transaction = client.createClientTransaction(&clientHandler);
    assert(transaction);

    transaction->setContext(context);

    // Build the request

    error = HttpEchoClientRequestBuilder(host, port, absPath, method,
                                         contentType, transaction);
    assert(ESB_SUCCESS == error);

    // Send the request (asynch) - the context will resubmit the request for
    // <iteration> - 1 iterations.

    error = client.executeClientTransaction(transaction);
    assert(ESB_SUCCESS == error);
  }

  while (!clientHandler.isFinished()) {
    sleep(1);
  }

  // Stop

  error = client.stop();
  assert(ESB_SUCCESS == error);
  error = server.stop();
  assert(ESB_SUCCESS == error);

  // TODO assert on counters here
  client.counters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  server.counters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);

  // Destroy

  client.destroy();
  server.destroy();

  ESB::Time::Instance().stop();
  error = ESB::Time::Instance().join();
  assert(ESB_SUCCESS == error);

  return ESB_SUCCESS;
}
