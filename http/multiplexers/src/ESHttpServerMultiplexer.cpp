#ifndef ES_HTTP_SERVER_MULTIPLEXER_H
#include <ESHttpServerMultiplexer.h>
#endif

namespace ES {

HttpServerMultiplexer::HttpServerMultiplexer(ESB::UInt32 maxSockets,
                                             HttpServerHandler &serverHandler,
                                             HttpServerCounters &serverCounters)
    : HttpMultiplexer(maxSockets),
      _serverSocketFactory(serverHandler, serverCounters, _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverStack(_factoryAllocator, _multiplexer, _ioBufferPool,
                   serverHandler, serverCounters, _serverTransactionFactory,
                   _serverSocketFactory),
      _serverCommandSocket(_serverStack) {
  _serverSocketFactory.setStack(_serverStack);
}

HttpServerMultiplexer::~HttpServerMultiplexer() {}

bool HttpServerMultiplexer::run(ESB::SharedInt *isRunning) {
  ESB::Error error = _multiplexer.addMultiplexedSocket(&_serverCommandSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add command socket to multiplexer");
    return false;
  }

  return _multiplexer.run(isRunning);
}

const char *HttpServerMultiplexer::name() const { return _multiplexer.name(); }

ESB::CleanupHandler *HttpServerMultiplexer::cleanupHandler() { return NULL; }

HttpServerMultiplexer::HttpServerStackImpl::HttpServerStackImpl(
    ESB::Allocator &allocator, ESB::EpollMultiplexer &multiplexer,
    ESB::BufferPool &bufferPool, HttpServerHandler &handler,
    HttpServerCounters &counters,
    HttpServerTransactionFactory &transactionFactory,
    HttpServerSocketFactory &socketFactory)
    : _allocator(allocator),
      _multiplexer(multiplexer),
      _bufferPool(bufferPool),
      _handler(handler),
      _counters(counters),
      _transactionFactory(transactionFactory),
      _socketFactory(socketFactory) {}

HttpServerMultiplexer::HttpServerStackImpl::~HttpServerStackImpl() {}

bool HttpServerMultiplexer::HttpServerStackImpl::isRunning() {
  return _multiplexer.isRunning();
}

HttpServerTransaction *
HttpServerMultiplexer::HttpServerStackImpl::createServerTransaction() {
  return _transactionFactory.create();
}

void HttpServerMultiplexer::HttpServerStackImpl::destroyTransaction(
    HttpServerTransaction *transaction) {
  return _transactionFactory.release(transaction);
}

ESB::Buffer *HttpServerMultiplexer::HttpServerStackImpl::acquireBuffer() {
  return _bufferPool.acquireBuffer();
}

void HttpServerMultiplexer::HttpServerStackImpl::releaseBuffer(
    ESB::Buffer *buffer) {
  _bufferPool.releaseBuffer(buffer);
}

ESB::Error HttpServerMultiplexer::HttpServerStackImpl::addServerSocket(
    ESB::TCPSocket::State &state) {
  HttpServerSocket *socket = _socketFactory.create(state);

  if (!socket) {
    return ESB_OUT_OF_MEMORY;
  }

  ESB::Error error = _multiplexer.addMultiplexedSocket(socket);

  if (ESB_SUCCESS != error) {
    _socketFactory.release(socket);
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error HttpServerMultiplexer::HttpServerStackImpl::addListeningSocket(
    ESB::ListeningTCPSocket &socket) {
  HttpListeningSocket *listener = new (_allocator)
      HttpListeningSocket(*this, _handler, _allocator.cleanupHandler());

  if (!listener) {
    ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "Cannot create listener on %s",
                        socket.logAddress());
    return ESB_OUT_OF_MEMORY;
  }

  ESB::Error error = listener->initialize(socket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot initialize listener on %s",
                        socket.logAddress());
    _allocator.cleanupHandler().destroy(listener);
    return error;
  }

  error = _multiplexer.addMultiplexedSocket(listener);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot add listener on %s to multiplexer",
                        socket.logAddress());
    _allocator.cleanupHandler().destroy(listener);
    return error;
  }

  return ESB_SUCCESS;
}

HttpServerCounters &
HttpServerMultiplexer::HttpServerStackImpl::serverCounters() {
  return _counters;
}

}  // namespace ES
