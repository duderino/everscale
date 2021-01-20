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

#ifndef ESB_SIGNAL_HANDLER_H
#include <ESBSignalHandler.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace ES {

class HttpNullProxyHandler : public HttpProxyHandler {
 public:
  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                      ESB::UInt32 *bytesAvailable) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                        unsigned char *body, ESB::UInt32 bytesRequested) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                         const unsigned char *body, ESB::UInt32 bytesOffered,
                                         ESB::UInt32 *bytesConsumed) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                              HttpClientHandler::State state) {
    assert(!"should not be called");
  }

  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer, HttpServerStream &serverStream) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                        unsigned const char *body, ESB::UInt32 bytesOffered,
                                        ESB::UInt32 *bytesConsumed) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                       ESB::UInt32 *bytesAvailable) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                         unsigned char *body, ESB::UInt32 bytesRequested) {
    assert(!"should not be called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                              HttpServerHandler::State state) {
    assert(!"should not be called");
  }
};

static HttpNullProxyHandler NullProxyHandler;
static ESB::ListeningSocket NullListener("null-listener");

HttpIntegrationTest::HttpIntegrationTest(const HttpTestParams &testParams, ESB::ListeningSocket &originListener,
                                         ESB::ListeningSocket &_proxyListener, HttpClientHandler &clientHandler,
                                         HttpProxyHandler &proxyHandler, HttpServerHandler &serverHandler)
    : _params(testParams),
      _proxyListener(_proxyListener),
      _originListener(originListener),
      _clientHandler(clientHandler),
      _proxyHandler(proxyHandler),
      _originHandler(serverHandler),
      _client("load", _params.clientThreads(), _clientHandler),
      _proxy("prox", _params.proxyThreads(), _proxyHandler),
      _origin("orig", _params.originThreads(), _originHandler) {
  HttpClientSocket::SetReuseConnections(_params.reuseConnections());
}

HttpIntegrationTest::HttpIntegrationTest(const HttpTestParams &testParams, ESB::ListeningSocket &originListener,
                                         HttpClientHandler &clientHandler, HttpServerHandler &serverHandler)
    : HttpIntegrationTest(testParams, originListener, NullListener, clientHandler, NullProxyHandler, serverHandler) {
  assert(0 == _params.proxyThreads());
}

HttpIntegrationTest::~HttpIntegrationTest() {
  _client.destroy();
  _origin.destroy();
  if (0 < _params.proxyThreads()) {
    _proxy.destroy();
  }
}

ESB::Error HttpIntegrationTest::run() {
  const ESB::UInt32 totalTransactions = _params.connections() * _params.requestsPerConnection();
  HttpLoadgenContext::SetTotalIterations(totalTransactions);

  ESB::Error error = ESB::SignalHandler::Instance().initialize();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot install signal handlers");
    return error;
  }

  error = ESB::Time::Instance().start();
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot start time thread");
    return error;
  }

  //
  // Max out open files
  //

  error = ESB::SystemConfig::Instance().setSocketSoftMax(ESB::SystemConfig::Instance().socketHardMax());
  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "cannot raise max fd limit");
    return error;
  }

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

  ESB::SocketAddress originAddress(
      _params.destinationAddress(), _originListener.listeningAddress().port(),
      _params.secure() ? ESB::SocketAddress::TransportType::TLS : ESB::SocketAddress::TransportType::TCP);
  ESB::SocketAddress proxyAddress(
      _params.destinationAddress(), _proxyListener.listeningAddress().port(),
      _params.secure() ? ESB::SocketAddress::TransportType::TLS : ESB::SocketAddress::TransportType::TCP);
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

  while (ESB::SignalHandler::Instance().running() && !HttpLoadgenContext::IsFinished()) {
    sleep(1);
  }

  ESB_LOG_NOTICE("[test] finished");

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

  _client.clientCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Warning);
  _origin.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Warning);

  if (0 < _params.proxyThreads()) {
    _proxy.serverCounters().log(ESB::Logger::Instance(), ESB::Logger::Severity::Warning);
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
