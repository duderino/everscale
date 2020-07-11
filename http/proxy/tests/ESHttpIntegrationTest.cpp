#ifndef ES_HTTP_INTEGRATION_TEST_H
#include "ESHttpIntegrationTest.h"
#endif

#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ES_HTTP_LOADGEN_SEED_COMMAND_H
#include <ESHttpLoadgenSeedCommand.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ES {

static volatile ESB::Word IsRunning = 1;
static void SignalHandler(int signal) { IsRunning = 0; }

class HttpNullProxyHandler : public HttpProxyHandler {
 public:
  ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                              ESB::UInt32 *bytesAvailable) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream, unsigned char *body,
                                ESB::UInt32 bytesRequested) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                 const unsigned char *body, ESB::UInt32 bytesOffered,
                                 ESB::UInt32 *bytesConsumed) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                      HttpClientHandler::State state) override {
    assert(!"should not be called");
  }

  ESB::Error acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream, unsigned const char *body,
                                ESB::UInt32 bytesOffered, ESB::UInt32 *bytesConsumed) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                               ESB::UInt32 *bytesAvailable) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream, unsigned char *body,
                                 ESB::UInt32 bytesRequested) override {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  void endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                      HttpServerHandler::State state) override {
    assert(!"should not be called");
  }
};

static HttpNullProxyHandler NullProxyHandler;
static ESB::ListeningTCPSocket NullListener("null-listener");

HttpIntegrationTest::HttpIntegrationTest(HttpTestParams &testParams, ESB::ListeningTCPSocket &originListener,
                                         ESB::ListeningTCPSocket &_proxyListener, HttpClientHandler &clientHandler,
                                         HttpProxyHandler &proxyHandler, HttpServerHandler &serverHandler)
    : _params(testParams),
      _logger(stdout),
      _proxyListener(_proxyListener),
      _originListener(originListener),
      _clientHandler(clientHandler),
      _proxyHandler(proxyHandler),
      _originHandler(serverHandler),
      _client("loadgen", _params.clientThreads(), _clientHandler),
      _proxy("proxy", _params.proxyThreads(), _proxyHandler),
      _origin("origin", _params.originThreads(), _originHandler) {
  HttpClientSocket::SetReuseConnections(_params.reuseConnections());
}

HttpIntegrationTest::HttpIntegrationTest(HttpTestParams &testParams, ESB::ListeningTCPSocket &originListener,
                                         HttpClientHandler &clientHandler, HttpServerHandler &serverHandler)
    : HttpIntegrationTest(testParams, originListener, NullListener, clientHandler, NullProxyHandler, serverHandler) {
  _params.proxyThreads(0);
}

HttpIntegrationTest::~HttpIntegrationTest() {
  _client.destroy();
  _origin.destroy();
  if (0 < _params.proxyThreads()) {
    _proxy.destroy();
  }
}

ESB::Error HttpIntegrationTest::run() {
  const ESB::UInt32 totalTransactions = _params.connections() * _params.iterations();
  HttpLoadgenContext::SetTotalIterations(totalTransactions);

  ESB::Time::Instance().start();
  ESB::SimpleFileLogger logger(stdout);
  logger.setSeverity(_params.logLevel());
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

  ESB::Error error = ESB::SystemConfig::Instance().setSocketSoftMax(ESB::SystemConfig::Instance().socketHardMax());
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot raise max fd limit");
    return error;
  }

  ESB_LOG_NOTICE("maximum sockets %u", ESB::SystemConfig::Instance().socketSoftMax());

  //
  // Init client, server, and proxy
  //

  error = _origin.initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot init origin server");
    return error;
  }

  error = _client.initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot init client");
    return error;
  }

  if (0 < _params.proxyThreads()) {
    error = _proxy.initialize();
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot init proxy");
      return error;
    }
  }

  //
  // Start client, server, and proxy
  //

  error = _origin.start();

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot start origin server");
    return error;
  }

  error = _origin.addListener(_originListener);
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot add origin listener");
    return error;
  }

  if (0 < _params.proxyThreads()) {
    error = _proxy.start();
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot start proxy");
      return error;
    }

    error = _proxy.addListener(_proxyListener);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot add proxy listener");
      return error;
    }
  }

  error = _client.start();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot start client");
    return error;
  }

  // add load generators to running client

  ESB::SocketAddress originAddress(_params.destinationAddress(), _originListener.listeningAddress().port(),
                                   ESB::SocketAddress::TransportType::TCP);
  ESB::SocketAddress proxyAddress(_params.destinationAddress(), _proxyListener.listeningAddress().port(),
                                  ESB::SocketAddress::TransportType::TCP);
  ESB::SocketAddress &clientDestination = 0 < _params.proxyThreads() ? proxyAddress : originAddress;

  for (int i = 0; i < _client.threads(); ++i) {
    HttpLoadgenSeedCommand *command = new (ESB::SystemAllocator::Instance())
        HttpLoadgenSeedCommand(clientDestination, _params, ESB::SystemAllocator::Instance().cleanupHandler());
    error = _client.push(command, i);
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot push seed command");
      return error;
    }
  }

  //
  // Wait for all requests to finish
  //

  while (IsRunning && !HttpLoadgenContext::IsFinished()) {
    sleep(1);
  }

  ESB_LOG_NOTICE("test finished");

  //
  // Stop client, server, and proxy
  //

  error = _client.stop();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot stop client");
    return error;
  }

  if (0 < _params.proxyThreads()) {
    error = _proxy.stop();
    if (ESB_SUCCESS != error) {
      ESB_LOG_CRITICAL_ERRNO(error, "cannot stop proxy");
      return error;
    }
  }

  error = _origin.stop();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot stop origin server");
    return error;
  }

  // Dump performance metrics

  _client.clientCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  _origin.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);

  if (0 < _params.proxyThreads()) {
    _proxy.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Notice);
  }

  ESB::Time::Instance().stop();
  error = ESB::Time::Instance().join();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot stop time thread");
    return error;
  }

  return ESB_SUCCESS;
}

}  // namespace ES
