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
};

class HttpNullClientCounters : public HttpClientCounters {
 public:
  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
    assert(0 == "HttpNullClientCounters called");
  }

  virtual ESB::PerformanceCounter *getSuccesses() {
    assert(0 == "HttpNullClientCounters called");
    return NULL;
  }

  virtual const ESB::PerformanceCounter *getSuccesses() const {
    assert(0 == "HttpNullClientCounters called");
    return NULL;
  }

  virtual ESB::PerformanceCounter *getFailures() {
    assert(0 == "HttpNullClientCounters called");
    return NULL;
  }

  virtual const ESB::PerformanceCounter *getFailures() const {
    assert(0 == "HttpNullClientCounters called");
    return NULL;
  }
};

class HttpNullServerCounters : public HttpServerCounters {
 public:
  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
    assert(0 == "HttpNullServerCounters called");
  }
  virtual ESB::PerformanceCounter *getSuccessfulTransactions() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getSuccessfulTransactions() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getRequestHeaderErrors() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getRequestHeaderErrors() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getRequestHeaderFailures() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getRequestHeaderFailures() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getRequestHeaderTimeouts() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getRequestHeaderTimeouts() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getRequestBodyErrors() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getRequestBodyErrors() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getRequestBodyFailures() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getRequestBodyFailures() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getRequestBodyTimeouts() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getRequestBodyTimeouts() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getResponseHeaderErrors() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getResponseHeaderErrors() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getResponseHeaderFailures() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getResponseHeaderFailures() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getResponseHeaderTimeouts() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getResponseHeaderTimeouts() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getResponseBodyErrors() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getResponseBodyErrors() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getResponseBodyFailures() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getResponseBodyFailures() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::PerformanceCounter *getResponseBodyTimeouts() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::PerformanceCounter *getResponseBodyTimeouts() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::SharedInt *getTotalConnections() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::SharedInt *getTotalConnections() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual ESB::SharedAveragingCounter *getAverageTransactionsPerConnection() {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
  virtual const ESB::SharedAveragingCounter *getAverageTransactionsPerConnection() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
};

static HttpNullClientHandler HttpNullClientHandler;
static HttpNullServerHandler HttpNullServerHandler;
static HttpNullClientCounters HttpNullClientCounters;
static HttpNullServerCounters HttpNullServerCounters;
static ESB::ClientTLSContextIndex EmptyClientContextIndex(0, 0, ESB::SystemAllocator::Instance());
static ESB::ServerTLSContextIndex EmptyServerContextIndex(0, 0, ESB::SystemAllocator::Instance());

HttpProxyMultiplexer::HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                                           HttpClientHandler &clientHandler, HttpServerHandler &serverHandler,
                                           HttpClientCounters &clientCounters, HttpServerCounters &serverCounters,
                                           ESB::ClientTLSContextIndex &clientContextIndex,
                                           ESB::ServerTLSContextIndex &serverContextIndex)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(), ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0, ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(ESB_PAGE_SIZE * 1000 - ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                        ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(namePrefix, idleTimeoutMsec, maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, serverHandler, serverCounters, serverContextIndex, _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(namePrefix, *this),
      _clientSocketFactory(*this, clientHandler, clientCounters, clientContextIndex, _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(namePrefix, *this),
      _clientHandler(clientHandler),
      _serverHandler(serverHandler),
      _clientCounters(clientCounters),
      _serverCounters(serverCounters) {}

HttpProxyMultiplexer::HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                                           HttpClientHandler &clientHandler, HttpClientCounters &clientCounters,
                                           ESB::ClientTLSContextIndex &clientContextIndex)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(), ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0, ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(ESB_PAGE_SIZE * 1000 - ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                        ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(namePrefix, idleTimeoutMsec, maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, HttpNullServerHandler, HttpNullServerCounters, EmptyServerContextIndex,
                           _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(namePrefix, *this),
      _clientSocketFactory(*this, clientHandler, clientCounters, clientContextIndex, _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(namePrefix, *this),
      _clientHandler(clientHandler),
      _serverHandler(HttpNullServerHandler),
      _clientCounters(clientCounters),
      _serverCounters(HttpNullServerCounters) {}

HttpProxyMultiplexer::HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                                           HttpServerHandler &serverHandler, HttpServerCounters &serverCounters,
                                           ESB::ServerTLSContextIndex &serverContextIndex)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(), ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0, ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(ESB_PAGE_SIZE * 1000 - ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
                        ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(namePrefix, idleTimeoutMsec, maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, serverHandler, serverCounters, serverContextIndex, _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(namePrefix, *this),
      _clientSocketFactory(*this, HttpNullClientHandler, HttpNullClientCounters, EmptyClientContextIndex,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(namePrefix, *this),
      _clientHandler(HttpNullClientHandler),
      _serverHandler(serverHandler),
      _clientCounters(HttpNullClientCounters),
      _serverCounters(serverCounters) {}

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

HttpServerCounters &HttpProxyMultiplexer::serverCounters() { return _serverCounters; }

HttpServerTransaction *HttpProxyMultiplexer::createServerTransaction() { return _serverTransactionFactory.create(); }

void HttpProxyMultiplexer::destroyServerTransaction(HttpServerTransaction *transaction) {
  _serverTransactionFactory.release(transaction);
}

ESB::SocketMultiplexer &HttpProxyMultiplexer::multiplexer() { return _multiplexer; }

bool HttpProxyMultiplexer::shutdown() { return !_multiplexer.isRunning(); }

}  // namespace ES
