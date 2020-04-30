#ifndef ES_HTTP_PROXY_MULTIPLEXER_H
#include <ESHttpProxyMultiplexer.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

class HttpNullClientHandler : public HttpClientHandler {
 public:
  virtual ESB::UInt32 reserveRequestChunk(HttpMultiplexer &multiplexer,
                                          HttpStream &stream) {
    assert(0 == "HttpNullClientHandler called");
    return 0;
  }

  virtual void fillRequestChunk(HttpMultiplexer &multiplexer,
                                HttpStream &stream, unsigned char *chunk,
                                ESB::UInt32 chunkSize) {
    assert(0 == "HttpNullClientHandler called");
  }

  virtual Result receiveResponseHeaders(HttpMultiplexer &multiplexer,
                                        HttpStream &stream) {
    assert(0 == "HttpNullClientHandler called");
    return ES_HTTP_CLIENT_HANDLER_CLOSE;
  }

  virtual ESB::UInt32 reserveResponseChunk(HttpMultiplexer &multiplexer,
                                           HttpStream &stream) {
    assert(0 == "HttpNullClientHandler called");
    return 0;
  }

  virtual void receivePaused(HttpMultiplexer &multiplexer, HttpStream &stream) {
    assert(0 == "HttpNullClientHandler called");
  }

  virtual Result receiveResponseChunk(HttpMultiplexer &multiplexer,
                                      HttpStream &stream,
                                      unsigned const char *chunk,
                                      ESB::UInt32 chunkSize) {
    assert(0 == "HttpNullClientHandler called");
    return ES_HTTP_CLIENT_HANDLER_CLOSE;
  }

  virtual void endClientTransaction(HttpMultiplexer &multiplexer,
                                    HttpStream &stream, State state) {
    assert(0 == "HttpNullClientHandler called");
  }
};

class HttpNullServerHandler : public HttpServerHandler {
 public:
  virtual Result acceptConnection(HttpMultiplexer &stack,
                                  ESB::SocketAddress *address) {
    assert(0 == "HttpNullServerHandler called");
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  virtual Result beginServerTransaction(HttpMultiplexer &stack,
                                        HttpStream &stream) {
    assert(0 == "HttpNullServerHandler called");
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  virtual Result receiveRequestHeaders(HttpMultiplexer &stack,
                                       HttpStream &stream) {
    assert(0 == "HttpNullServerHandler called");
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  virtual ESB::UInt32 reserveRequestChunk(HttpMultiplexer &stack,
                                          HttpStream &stream) {
    assert(0 == "HttpNullServerHandler called");
    return 0;
  }

  virtual Result receiveRequestChunk(HttpMultiplexer &stack, HttpStream &stream,
                                     unsigned const char *chunk,
                                     ESB::UInt32 chunkSize) {
    assert(0 == "HttpNullServerHandler called");
    return ES_HTTP_SERVER_HANDLER_CLOSE;
  }

  virtual void receivePaused(HttpMultiplexer &stack, HttpStream &stream) {
    assert(0 == "HttpNullServerHandler called");
  }

  virtual ESB::UInt32 reserveResponseChunk(HttpMultiplexer &stack,
                                           HttpStream &stream) {
    assert(0 == "HttpNullServerHandler called");
    return 0;
  }

  virtual void fillResponseChunk(HttpMultiplexer &stack, HttpStream &stream,
                                 unsigned char *chunk, ESB::UInt32 chunkSize) {
    assert(0 == "HttpNullServerHandler called");
  }

  virtual void endServerTransaction(HttpMultiplexer &stack, HttpStream &stream,
                                    State state) {
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
  virtual const ESB::SharedAveragingCounter *
  getAverageTransactionsPerConnection() const {
    assert(0 == "HttpNullServerCounters called");
    return NULL;
  }
};

static HttpNullClientHandler HttpNullClientHandler;
static HttpNullServerHandler HttpNullServerHandler;
static HttpNullClientCounters HttpNullClientCounters;
static HttpNullServerCounters HttpNullServerCounters;

HttpProxyMultiplexer::HttpProxyMultiplexer(ESB::UInt32 maxSockets,
                                           HttpClientHandler &clientHandler,
                                           HttpServerHandler &serverHandler,
                                           HttpClientCounters &clientCounters,
                                           HttpServerCounters &serverCounters)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(),
                             ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0,
                    ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(
          ESB_PAGE_SIZE * 1000 -
              ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
          ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, serverHandler, serverCounters,
                           _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(*this),
      _clientSocketFactory(*this, clientHandler, clientCounters,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(*this),
      _clientHandler(clientHandler),
      _serverHandler(serverHandler),
      _clientCounters(clientCounters),
      _serverCounters(serverCounters) {}

HttpProxyMultiplexer::HttpProxyMultiplexer(ESB::UInt32 maxSockets,
                                           HttpClientHandler &clientHandler,
                                           HttpClientCounters &clientCounters)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(),
                             ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0,
                    ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(
          ESB_PAGE_SIZE * 1000 -
              ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
          ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, HttpNullServerHandler, HttpNullServerCounters,
                           _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(*this),
      _clientSocketFactory(*this, clientHandler, clientCounters,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(*this),
      _clientHandler(clientHandler),
      _serverHandler(HttpNullServerHandler),
      _clientCounters(clientCounters),
      _serverCounters(HttpNullServerCounters) {}

HttpProxyMultiplexer::HttpProxyMultiplexer(ESB::UInt32 maxSockets,
                                           HttpServerHandler &serverHandler,
                                           HttpServerCounters &serverCounters)
    : _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(),
                             ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE,
                             ESB::SystemAllocator::Instance()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize(), 0,
                    ESB::NullLock::Instance(), _ioBufferPoolAllocator),
      _factoryAllocator(
          ESB_PAGE_SIZE * 1000 -
              ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE),
          ESB_CACHE_LINE_SIZE, ESB_PAGE_SIZE, ESB::SystemAllocator::Instance()),
      _multiplexer(maxSockets, ESB::SystemAllocator::Instance()),
      _serverSocketFactory(*this, serverHandler, serverCounters,
                           _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverCommandSocket(*this),
      _clientSocketFactory(*this, HttpNullClientHandler, HttpNullClientCounters,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientCommandSocket(*this),
      _clientHandler(HttpNullClientHandler),
      _serverHandler(serverHandler),
      _clientCounters(HttpNullClientCounters),
      _serverCounters(serverCounters) {}

HttpProxyMultiplexer::~HttpProxyMultiplexer() {}

ESB::Error HttpProxyMultiplexer::addMultiplexedSocket(
    ESB::MultiplexedSocket *multiplexedSocket) {
  return _multiplexer.addMultiplexedSocket(multiplexedSocket);
}

ESB::Error HttpProxyMultiplexer::updateMultiplexedSocket(
    ESB::MultiplexedSocket *socket) {
  return _multiplexer.updateMultiplexedSocket(socket);
}

ESB::Error HttpProxyMultiplexer::removeMultiplexedSocket(
    ESB::MultiplexedSocket *socket, bool removeFromList) {
  return _multiplexer.removeMultiplexedSocket(socket, removeFromList);
}

int HttpProxyMultiplexer::currentSockets() const {
  return _multiplexer.currentSockets();
}

int HttpProxyMultiplexer::maximumSockets() const {
  return _multiplexer.maximumSockets();
}

bool HttpProxyMultiplexer::isRunning() const {
  return _multiplexer.isRunning();
}

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

HttpClientTransaction *HttpProxyMultiplexer::createClientTransaction() {
  return _clientTransactionFactory.create();
}

ESB::Error HttpProxyMultiplexer::executeTransaction(
    HttpClientTransaction *transaction) {
  return _clientSocketFactory.executeClientTransaction(transaction);
}

void HttpProxyMultiplexer::destroyTransaction(
    HttpClientTransaction *transaction) {
  return _clientTransactionFactory.release(transaction);
}

ESB::Buffer *HttpProxyMultiplexer::acquireBuffer() {
  return _ioBufferPool.acquireBuffer();
}

void HttpProxyMultiplexer::releaseBuffer(ESB::Buffer *buffer) {
  _ioBufferPool.releaseBuffer(buffer);
}

ESB::Error HttpProxyMultiplexer::addServerSocket(ESB::TCPSocket::State &state) {
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

ESB::Error HttpProxyMultiplexer::addListeningSocket(
    ESB::ListeningTCPSocket &socket) {
  HttpListeningSocket *listener = new (_factoryAllocator) HttpListeningSocket(
      *this, _serverHandler, _factoryAllocator.cleanupHandler());

  if (!listener) {
    ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create listener on %s",
                        socket.logAddress());
    return ESB_OUT_OF_MEMORY;
  }

  ESB::Error error = listener->initialize(socket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot initialize listener on %s",
                        socket.logAddress());
    _factoryAllocator.cleanupHandler().destroy(listener);
    return error;
  }

  error = _multiplexer.addMultiplexedSocket(listener);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot add listener on %s to multiplexer",
                        socket.logAddress());
    _factoryAllocator.cleanupHandler().destroy(listener);
    return error;
  }

  return ESB_SUCCESS;
}

HttpServerCounters &HttpProxyMultiplexer::serverCounters() {
  return _serverCounters;
}

HttpServerTransaction *HttpProxyMultiplexer::createServerTransaction() {
  return _serverTransactionFactory.create();
}

void HttpProxyMultiplexer::destroyTransaction(
    HttpServerTransaction *transaction) {
  _serverTransactionFactory.release(transaction);
}

ESB::SocketMultiplexer &HttpProxyMultiplexer::multiplexer() {
  return _multiplexer;
}

bool HttpProxyMultiplexer::shutdown() { return !_multiplexer.isRunning(); }

}  // namespace ES
