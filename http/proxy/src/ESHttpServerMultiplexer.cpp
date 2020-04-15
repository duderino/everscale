#ifndef ES_HTTP_SERVER_MULTIPLEXER_H
#include <ESHttpServerMultiplexer.h>
#endif

namespace ES {

HttpServerMultiplexer::HttpServerMultiplexer(
    ESB::UInt32 maxSockets, ESB::ListeningTCPSocket &listeningSocket,
    HttpServerHandler &serverHandler, HttpServerCounters &serverCounters)
    : HttpMultiplexer(maxSockets),
      _serverSocketFactory(serverHandler, serverCounters, _factoryAllocator),
      _serverTransactionFactory(_factoryAllocator),
      _serverStack(_multiplexer, _ioBufferPool, serverCounters,
                   _serverTransactionFactory, _serverSocketFactory),
      _listeningSocket(listeningSocket, _serverStack, serverHandler) {
  _serverSocketFactory.setStack(_serverStack);
}

HttpServerMultiplexer::~HttpServerMultiplexer() {}

bool HttpServerMultiplexer::run(ESB::SharedInt *isRunning) {
  ESB::Error error = addMultiplexedSocket(&_listeningSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add listener to multiplexer");
    return false;
  }

  return _multiplexer.run(isRunning);
}

const char *HttpServerMultiplexer::name() const { return _multiplexer.name(); }

ESB::CleanupHandler *HttpServerMultiplexer::cleanupHandler() { return NULL; }

HttpServerMultiplexer::HttpServerStackImpl::HttpServerStackImpl(
    ESB::EpollMultiplexer &multiplexer, ESB::BufferPool &bufferPool,
    HttpServerCounters &counters,
    HttpServerTransactionFactory &transactionFactory,
    HttpServerSocketFactory &socketFactory)
    : _multiplexer(multiplexer),
      _bufferPool(bufferPool),
      _counters(counters),
      _transactionFactory(transactionFactory),
      _socketFactory(socketFactory) {}

HttpServerMultiplexer::HttpServerStackImpl::~HttpServerStackImpl() {}

bool HttpServerMultiplexer::HttpServerStackImpl::isRunning() {
  return _multiplexer.isRunning();
}

HttpServerTransaction *
HttpServerMultiplexer::HttpServerStackImpl::createTransaction() {
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

HttpServerCounters &HttpServerMultiplexer::HttpServerStackImpl::counters() {
  return _counters;
}

}  // namespace ES
