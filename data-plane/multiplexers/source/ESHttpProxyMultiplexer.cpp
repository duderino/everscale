#ifndef ES_HTTP_PROXY_MULTIPLEXER_H
#include <ESHttpProxyMultiplexer.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

class HttpNullClientHandler : public HttpClientHandler {
 public:
  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream) {
    assert(!"function should not be called");
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
    assert(0 == "HttpNullClientHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                      ESB::UInt64 *bytesAvailable) {
    assert(0 == "HttpNullClientHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream, unsigned char *chunk,
                                        ESB::UInt64 bytesRequested) {
    assert(0 == "HttpNullClientHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                         const unsigned char *chunk, ESB::UInt64 chunkSize,
                                         ESB::UInt64 *bytesConsumed) {
    assert(0 == "HttpNullClientHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &stream) {
    assert(0 == "HttpNullClientHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &stream, State state) {
    assert(0 == "HttpNullClientHandler called");
  }

  virtual void dumpClientCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const {}
};

class HttpNullServerHandler : public HttpServerHandler {
 public:
  virtual ESB::Error acceptConnection(HttpMultiplexer &stack, ESB::SocketAddress *address) {
    assert(0 == "HttpNullServerHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error beginTransaction(HttpMultiplexer &stack, HttpServerStream &stream) {
    assert(0 == "HttpNullServerHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &stack, HttpServerStream &stream) {
    assert(0 == "HttpNullServerHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                        unsigned const char *chunk, ESB::UInt64 chunkSize, ESB::UInt64 *bytesConsumed) {
    assert(0 == "HttpNullServerHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                       ESB::UInt64 *bytesAvailable) {
    assert(0 == "HttpNullServerHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream, unsigned char *chunk,
                                         ESB::UInt64 bytesRequested) {
    assert(0 == "HttpNullServerHandler called");
    return ESB_NOT_IMPLEMENTED;
  }

  virtual void endTransaction(HttpMultiplexer &stack, HttpServerStream &stream, State state) {
    assert(0 == "HttpNullServerHandler called");
  }

  virtual void dumpServerCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const {}
};

// class HttpNullClientCounters : public HttpClientCounters {
//  public:
//   virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
//     assert(0 == "HttpNullClientCounters called");
//   }
//
//   virtual ESB::PerformanceCounter &successfulTransactions() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &successfulTransactions() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &failedTransactions() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &failedTransactions() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestBeginError() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestBeginError() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestResolveError() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestResolveError() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestConnectError() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestConnectError() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestHeaderSendError() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestHeaderSendError() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestBodySendError() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &requestBodySendError() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseHeaderReceiveError() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseHeaderReceiveError() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseBodyReceiveError() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseBodyReceiveError() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus1xx() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus1xx() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus200() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus200() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus201() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus201() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus202() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus202() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus204() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus204() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus2xx() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus2xx() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus304() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus304() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus3xx() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus3xx() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus400() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus400() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus401() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus401() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus403() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus403() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus404() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus404() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus410() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus410() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus4xx() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus4xx() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus500() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus500() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus502() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus502() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus503() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus503() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus504() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus504() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus5xx() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus5xx() const { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatusOther() { assert(0 == "HttpNullClientCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatusOther() const { assert(0 == "HttpNullClientCounters called"); }
// };
//
// class HttpNullServerCounters : public HttpServerCounters {
//  public:
//   virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
//     assert(0 == "HttpNullServerCounters called");
//   }
//   virtual ESB::PerformanceCounter &successfulTransactions() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &successfulTransactions() const {
//     assert(0 == "HttpNullServerCounters called");
//   }
//   virtual ESB::PerformanceCounter &failedTransactions() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &failedTransactions() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &requestHeaderBeginError() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &requestHeaderBeginError() const {
//     assert(0 == "HttpNullServerCounters called");
//   }
//   virtual ESB::PerformanceCounter &requestHeaderReceiveError() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &requestHeaderReceiveError() const {
//     assert(0 == "HttpNullServerCounters called");
//   }
//   virtual ESB::PerformanceCounter &requestBodyReceiveError() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &requestBodyReceiveError() const {
//     assert(0 == "HttpNullServerCounters called");
//   }
//   virtual ESB::PerformanceCounter &requestBodySizeError() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &requestBodySizeError() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseHeaderSendError() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseHeaderSendError() const {
//     assert(0 == "HttpNullServerCounters called");
//   }
//   virtual ESB::PerformanceCounter &responseBodySendError() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseBodySendError() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus1xx() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus1xx() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus200() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus200() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus201() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus201() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus202() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus202() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus204() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus204() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus2xx() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus2xx() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus304() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus304() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus3xx() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus3xx() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus400() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus400() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus401() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus401() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus403() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus403() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus404() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus404() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus410() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus410() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus4xx() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus4xx() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus500() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus500() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus502() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus502() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus503() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus503() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus504() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus504() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatus5xx() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatus5xx() const { assert(0 == "HttpNullServerCounters called"); }
//   virtual ESB::PerformanceCounter &responseStatusOther() { assert(0 == "HttpNullServerCounters called"); }
//   virtual const ESB::PerformanceCounter &responseStatusOther() const { assert(0 == "HttpNullServerCounters called"); }
// };
//   static HttpNullClientCounters HttpNullClientCounters;
//   static HttpNullServerCounters HttpNullServerCounters;

static HttpNullClientHandler HttpNullClientHandler;
static HttpNullServerHandler HttpNullServerHandler;
static ESB::ClientTLSContextIndex EmptyClientContextIndex(0, 0, ESB::SystemAllocator::Instance());
static ESB::ServerTLSContextIndex EmptyServerContextIndex(0, 0, ESB::SystemAllocator::Instance());

HttpProxyMultiplexer::HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                                           HttpClientHandler &clientHandler, HttpServerHandler &serverHandler,
                                           ESB::ClientTLSContextIndex &clientContextIndex,
                                           ESB::ServerTLSContextIndex &serverContextIndex)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(), ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0, ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(ESB_PAGE_SIZE * 1000 - ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                        ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(namePrefix, idleTimeoutMsec, maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, serverHandler, _serverConnectionMetrics, serverContextIndex, _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(namePrefix, *this),
      _clientSocketFactory(*this, clientHandler, _clientConnectionMetrics, clientContextIndex, _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(namePrefix, *this),
      _clientHandler(clientHandler),
      _serverHandler(serverHandler),
      _isClient(true),
      _isServer(true) {}

HttpProxyMultiplexer::HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                                           HttpClientHandler &clientHandler,
                                           ESB::ClientTLSContextIndex &clientContextIndex)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(), ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0, ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(ESB_PAGE_SIZE * 1000 - ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                        ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(namePrefix, idleTimeoutMsec, maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, HttpNullServerHandler, _serverConnectionMetrics, EmptyServerContextIndex,
                           _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(namePrefix, *this),
      _clientSocketFactory(*this, clientHandler, _clientConnectionMetrics, clientContextIndex, _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(namePrefix, *this),
      _clientHandler(clientHandler),
      _serverHandler(HttpNullServerHandler),
      _isClient(true),
      _isServer(false) {}

HttpProxyMultiplexer::HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                                           HttpServerHandler &serverHandler,
                                           ESB::ServerTLSContextIndex &serverContextIndex)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(), ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0, ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(ESB_PAGE_SIZE * 1000 - ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                        ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(namePrefix, idleTimeoutMsec, maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, serverHandler, _serverConnectionMetrics, serverContextIndex, _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(namePrefix, *this),
      _clientSocketFactory(*this, HttpNullClientHandler, _clientConnectionMetrics, EmptyClientContextIndex,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(namePrefix, *this),
      _clientHandler(HttpNullClientHandler),
      _serverHandler(serverHandler),
      _isClient(false),
      _isServer(true) {}

HttpProxyMultiplexer::~HttpProxyMultiplexer() {}

ESB::Error HttpProxyMultiplexer::addMultiplexedSocket(ESB::MultiplexedSocket *multiplexedSocket) {
  return _multiplexer.addMultiplexedSocket(multiplexedSocket);
}

ESB::Error HttpProxyMultiplexer::updateMultiplexedSocket(ESB::MultiplexedSocket *socket) {
  return _multiplexer.updateMultiplexedSocket(socket);
}

ESB::Error HttpProxyMultiplexer::removeMultiplexedSocket(ESB::MultiplexedSocket *socket) {
  return _multiplexer.removeMultiplexedSocket(socket);
}

int HttpProxyMultiplexer::currentSockets() const { return _multiplexer.currentSockets(); }

int HttpProxyMultiplexer::maximumSockets() const { return _multiplexer.maximumSockets(); }

bool HttpProxyMultiplexer::isRunning() const { return _multiplexer.isRunning(); }

bool HttpProxyMultiplexer::run(ESB::SharedInt *isRunning) {
  ESB::Error error = _multiplexer.addMultiplexedSocket(&_clientCommandSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add command socket to multiplexer");
    return false;
  }

  error = _multiplexer.addMultiplexedSocket(&_serverCommandSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add command socket to multiplexer");
    return false;
  }

  return _multiplexer.run(isRunning);
}

const char *HttpProxyMultiplexer::name() const { return _multiplexer.name(); }

ESB::CleanupHandler *HttpProxyMultiplexer::cleanupHandler() { return NULL; }

HttpClientTransaction *HttpProxyMultiplexer::createClientTransaction() { return _clientTransactionFactory.create(); }

ESB::Error HttpProxyMultiplexer::executeClientTransaction(HttpClientTransaction *transaction) {
  return _clientSocketFactory.executeClientTransaction(transaction);
}

void HttpProxyMultiplexer::destroyClientTransaction(HttpClientTransaction *transaction) {
  return _clientTransactionFactory.release(transaction);
}

ESB::Buffer *HttpProxyMultiplexer::acquireBuffer() { return _ioBufferPool.acquireBuffer(); }

void HttpProxyMultiplexer::releaseBuffer(ESB::Buffer *buffer) { _ioBufferPool.releaseBuffer(buffer); }

ESB::Error HttpProxyMultiplexer::addServerSocket(ESB::Socket::State &state) {
  HttpServerSocket *socket = _serverSocketFactory.create(state);

  if (!socket) {
    return ESB_OUT_OF_MEMORY;
  }

  ESB::Error error = _multiplexer.addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    _serverSocketFactory.release(socket);
    return error;
  }

  _serverConnectionMetrics.totalConnections().inc();
  return ESB_SUCCESS;
}

ESB::Error HttpProxyMultiplexer::addListeningSocket(ESB::ListeningSocket &socket) {
  HttpListeningSocket *listener =
      new (_factoryAllocator) HttpListeningSocket(*this, _serverHandler, _factoryAllocator.cleanupHandler());

  if (!listener) {
    ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create listener on %s", socket.name());
    return ESB_OUT_OF_MEMORY;
  }

  ESB::Error error = listener->initialize(socket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot initialize listener on %s", socket.name());
    _factoryAllocator.cleanupHandler().destroy(listener);
    return error;
  }

  error = _multiplexer.addMultiplexedSocket(listener);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot add listener on %s to multiplexer", socket.name());
    _factoryAllocator.cleanupHandler().destroy(listener);
    return error;
  }

  return ESB_SUCCESS;
}

HttpServerTransaction *HttpProxyMultiplexer::createServerTransaction() { return _serverTransactionFactory.create(); }

void HttpProxyMultiplexer::destroyServerTransaction(HttpServerTransaction *transaction) {
  _serverTransactionFactory.release(transaction);
}

ESB::SocketMultiplexer &HttpProxyMultiplexer::multiplexer() { return _multiplexer; }

void HttpProxyMultiplexer::dumpCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const {
  const bool isProxy = _isClient && _isServer;

  if (_isClient) {
    _clientHandler.dumpClientCounters(logger, severity);

    if (isProxy) {
      ESB_LOG(logger, severity, "PROXY CONNECTS: %u", _clientConnectionMetrics.totalConnections().get());
    } else {
      ESB_LOG(logger, severity, "CLIENT CONNECTS: %u", _clientConnectionMetrics.totalConnections().get());
    }

    _clientConnectionMetrics.averageTransactionsPerConnection().log(logger, severity,
                                                                    isProxy ? "PROXY "
                                                                            : "CLIENT "
                                                                              "AVG TRANS PER CLIENT CONNECTION");
  }

  if (_isServer) {
    _serverHandler.dumpServerCounters(logger, severity);

    if (isProxy) {
      ESB_LOG(logger, severity, "PROXY ACCEPTS: %u", _serverConnectionMetrics.totalConnections().get());
    } else {
      ESB_LOG(logger, severity, "ORIGIN ACCEPTS: %u", _serverConnectionMetrics.totalConnections().get());
    }

    _serverConnectionMetrics.averageTransactionsPerConnection().log(logger, severity,
                                                                    isProxy ? "PROXY "
                                                                            : "ORIGIN "
                                                                              "AVG TRANS PER SERVER CONNECTION");
  }
}

bool HttpProxyMultiplexer::shutdown() { return !_multiplexer.isRunning(); }

}  // namespace ES
