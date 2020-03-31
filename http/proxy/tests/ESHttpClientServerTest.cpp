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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ES {
class SeedTransactions : public HttpSeedTransactionHandler {
 public:
  SeedTransactions(ESB::Allocator &allocator, ESB::UInt32 iterations,
                   ESB::Int32 port, const char *host, const char *absPath,
                   const char *method, const char *contentType)
      : _allocator(allocator),
        _iterations(iterations),
        _port(port),
        _host(host),
        _absPath(absPath),
        _method(method),
        _contentType(contentType) {}
  virtual ~SeedTransactions() {}

  virtual ESB::Error modifyTransaction(HttpClientTransaction *transaction) {
    // Create the request context
    HttpEchoClientContext *context =
        new (_allocator) HttpEchoClientContext(_iterations - 1, _allocator.cleanupHandler());
    assert(context);

    transaction->setContext(context);

    // Build the request

    ESB::Error error = HttpEchoClientRequestBuilder(
        _host, _port, _absPath, _method, _contentType, transaction);
    assert(ESB_SUCCESS == error);
    return error;
  }

 private:
  // Disabled
  SeedTransactions(const SeedTransactions &);
  SeedTransactions &operator=(const SeedTransactions &);

  ESB::Allocator &_allocator;
  const ESB::UInt32 _iterations;
  const ESB::Int32 _port;
  const char *_host;
  const char *_absPath;
  const char *_method;
  const char *_contentType;
};
}  // namespace ES

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

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity((ESB::Logger::Severity) logLevel);
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
  //ESB::DiscardAllocator allocator(ESB::SystemConfig::Instance().pageSize());
  SeedTransactions seed(ESB::SystemAllocator::Instance(), iterations, port, host, absPath, method, contentType);
  HttpClient client(clientThreads, connections, seed, clientHandler);

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
