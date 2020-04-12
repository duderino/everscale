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
      _serverStack(_epollMultiplexer, _serverTransactionFactory, _ioBufferPool),
      _listeningSocket(serverHandler, listeningSocket, _serverSocketFactory,
                       serverCounters) {
  _serverSocketFactory.setStack(_serverStack);
}

HttpServerMultiplexer::~HttpServerMultiplexer() {}

bool HttpServerMultiplexer::run(ESB::SharedInt *isRunning) {
  ESB::Error error = addMultiplexedSocket(&_listeningSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add listener to multiplexer");
    return false;
  }

  return _epollMultiplexer.run(isRunning);
}

const char *HttpServerMultiplexer::name() const {
  return _epollMultiplexer.name();
}

ESB::CleanupHandler *HttpServerMultiplexer::cleanupHandler() { return NULL; }

HttpServerMultiplexer::HttpServerStackImpl::HttpServerStackImpl(
    ESB::EpollMultiplexer &multiplexer,
    HttpServerTransactionFactory &serverTransactionFactory,
    ESB::BufferPool &bufferPool)
    : _bufferPool(bufferPool),
      _multiplexer(multiplexer),
      _serverTransactionFactory(serverTransactionFactory) {}

HttpServerMultiplexer::HttpServerStackImpl::~HttpServerStackImpl() {}

bool HttpServerMultiplexer::HttpServerStackImpl::isRunning() {
  return _multiplexer.isRunning();
}

HttpServerTransaction *
HttpServerMultiplexer::HttpServerStackImpl::createTransaction() {
  return _serverTransactionFactory.create();
}

void HttpServerMultiplexer::HttpServerStackImpl::destroyTransaction(
    HttpServerTransaction *transaction) {
  return _serverTransactionFactory.release(transaction);
}

ESB::Buffer *HttpServerMultiplexer::HttpServerStackImpl::acquireBuffer() {
  return _bufferPool.acquireBuffer();
}

void HttpServerMultiplexer::HttpServerStackImpl::releaseBuffer(
    ESB::Buffer *buffer) {
  _bufferPool.releaseBuffer(buffer);
}

}  // namespace ES
