#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ES_HTTP_CLIENT_H
#include <ESHttpClient.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_SIMPLE_FILE_LOGGER_H
#include <ESBSimpleFileLogger.h>
#endif

#ifndef ES_HTTP_ORIGIN_HANDLER_H
#include <ESHttpOriginHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ES_HTTP_LOADGEN_HANDLER_H
#include <ESHttpLoadgenHandler.h>
#endif

#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#include <ESHttpLoadgenSeedCommand.h>
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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

static volatile ESB::Word IsRunning = 1;
static void SignalHandler(int signal) { IsRunning = 0; }

using namespace ES;

int main(int argc, char **argv) {
  int clientThreads = 3;
  int serverThreads = 3;
  const char *destination = "127.0.0.1";
  const char *host = "localhost.localdomain";
  unsigned int connections = 500;  // concurrent connections
  unsigned int iterations = 500;   // http requests per concurrent connection
  bool reuseConnections = true;
  int logLevel = ESB::Logger::Notice;
  const char *method = "GET";
  const char *contentType = "octet-stream";
  const char *absPath = "/";
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

  const ESB::UInt32 totalTransactions = connections * iterations;
  HttpLoadgenContext::SetTotalIterations(totalTransactions);

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity((ESB::Logger::Severity)logLevel);
  ESB::Logger::SetInstance(&logger);

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

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(
      ESB::SystemConfig::Instance().socketHardMax());

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot raise max fd limit");
    return -1;
  }

  ESB_LOG_NOTICE("Maximum sockets %u",
                 ESB::SystemConfig::Instance().socketSoftMax());

  //
  // Create listening socket
  //

  // bind to port 0 so kernel will choose a free ephemeral port
  ESB::ListeningTCPSocket listener(0, ESB_UINT16_MAX);

  error = listener.bind();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot bind to port %u",
                           listener.listeningAddress().port());
    return -2;
  }

  ESB_LOG_NOTICE("Bound to port %u", listener.listeningAddress().port());

  ESB::SocketAddress destaddr(destination, listener.listeningAddress().port(),
                              ESB::SocketAddress::TransportType::TCP);

  //
  // Init client and server
  //

  HttpOriginHandler serverHandler;
  HttpServer server(serverThreads, serverHandler);

  error = server.initialize();

  if (ESB_SUCCESS != error) {
    return -4;
  }

  HttpLoadgenHandler clientHandler(absPath, method, contentType, body,
                                   sizeof(body));
  HttpClientSocket::SetReuseConnections(reuseConnections);
  HttpClient client(clientThreads, clientHandler);

  error = client.initialize();

  if (ESB_SUCCESS != error) {
    return -5;
  }

  //
  // Start client and server
  //

  error = server.start();

  if (ESB_SUCCESS != error) {
    return -6;
  }

  // add listening sockets to running server

  error = server.addListener(listener);

  if (ESB_SUCCESS != error) {
    return -7;
  }

  error = client.start();

  if (ESB_SUCCESS != error) {
    return -8;
  }

  // add load generators to running client

  for (int i = 0; i < client.threads(); ++i) {
    HttpLoadgenSeedCommand *command =
        new (ESB::SystemAllocator::Instance()) HttpLoadgenSeedCommand(
            connections / clientThreads, iterations, destaddr,
            listener.listeningAddress().port(), host, absPath, method,
            contentType, ESB::SystemAllocator::Instance().cleanupHandler());
    error = client.push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "Cannot push seed command");
      return -9;
    }
  }

  //
  // Wait for all requests to finish
  //

  while (IsRunning && !HttpLoadgenContext::IsFinished()) {
    sleep(1);
  }

  ESB_LOG_NOTICE("Load test finished");

  //
  // Stop client and server
  //

  error = client.stop();
  assert(ESB_SUCCESS == error);
  error = server.stop();
  assert(ESB_SUCCESS == error);

  // Dump performance metrics

  client.clientCounters().log(ESB::Logger::Instance(),
                              ESB::Logger::Severity::Notice);
  server.serverCounters().log(ESB::Logger::Instance(),
                              ESB::Logger::Severity::Notice);

  const ESB::UInt32 totalSuccesses =
      client.clientCounters().getSuccesses()->queries();
  const ESB::UInt32 totalFailures =
      client.clientCounters().getFailures()->queries();

  //
  // Destroy client and server
  //

  client.destroy();
  server.destroy();

  ESB::Time::Instance().stop();
  error = ESB::Time::Instance().join();
  assert(ESB_SUCCESS == error);

  //
  // Assert all requests succeeded
  //

  if (totalSuccesses != totalTransactions || 0 < totalFailures) {
    ESB_LOG_CRITICAL(
        "TEST FAILURE: expected %u successes but got %u successes and %u "
        "failures",
        totalTransactions, totalSuccesses, totalFailures);
    return -10;
  }

  return ESB_SUCCESS;
}
