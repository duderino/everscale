#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#include <ESHttpClientMultiplexer.h>
#endif

namespace ES {

HttpClientMultiplexer::HttpClientMultiplexer(ESB::UInt32 maxSockets,
                                             HttpClientHandler &clientHandler,
                                             HttpClientCounters &clientCounters)
    : HttpMultiplexer(maxSockets),
      _clientSocketFactory(*this, clientHandler, clientCounters,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientStack(_multiplexer, _clientSocketFactory,
                   _clientTransactionFactory, _ioBufferPool),
      _commandSocket(_clientStack) {
  _clientSocketFactory.setStack(_clientStack);
}

HttpClientMultiplexer::~HttpClientMultiplexer() {}

bool HttpClientMultiplexer::run(ESB::SharedInt *isRunning) {
  ESB::Error error = _multiplexer.addMultiplexedSocket(&_commandSocket);

  if (ESB_SUCCESS != error) {
    ESB_LOG_CRITICAL_ERRNO(error, "Cannot add command socket to multiplexer");
    return false;
  }

  return _multiplexer.run(isRunning);
}

const char *HttpClientMultiplexer::name() const { return _multiplexer.name(); }

ESB::CleanupHandler *HttpClientMultiplexer::cleanupHandler() { return NULL; }

HttpClientMultiplexer::HttpClientStackImpl::HttpClientStackImpl(
    ESB::EpollMultiplexer &multiplexer,
    HttpClientSocketFactory &clientSocketFactory,
    HttpClientTransactionFactory &clientTransactionFactory,
    ESB::BufferPool &bufferPool)
    : _bufferPool(bufferPool),
      _multiplexer(multiplexer),
      _clientSocketFactory(clientSocketFactory),
      _clientTransactionFactory(clientTransactionFactory) {}

HttpClientMultiplexer::HttpClientStackImpl::~HttpClientStackImpl() {}

bool HttpClientMultiplexer::HttpClientStackImpl::isRunning() {
  return _multiplexer.isRunning();
}

HttpClientTransaction *
HttpClientMultiplexer::HttpClientStackImpl::createTransaction() {
  return _clientTransactionFactory.create();
}

ESB::Error HttpClientMultiplexer::HttpClientStackImpl::executeClientTransaction(
    HttpClientTransaction *transaction) {
  return _clientSocketFactory.executeClientTransaction(transaction);
}

void HttpClientMultiplexer::HttpClientStackImpl::destroyTransaction(
    HttpClientTransaction *transaction) {
  return _clientTransactionFactory.release(transaction);
}

ESB::Buffer *HttpClientMultiplexer::HttpClientStackImpl::acquireBuffer() {
  return _bufferPool.acquireBuffer();
}

void HttpClientMultiplexer::HttpClientStackImpl::releaseBuffer(
    ESB::Buffer *buffer) {
  _bufferPool.releaseBuffer(buffer);
}

}  // namespace ES
