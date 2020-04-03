#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#include <ESHttpClientMultiplexer.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

HttpClientMultiplexer::HttpClientMultiplexer(
    ESB::UInt32 connections, HttpSeedTransactionHandler &seedTransactionHandler,
    ESB::UInt32 maxSockets, HttpClientHandler &handler,
    HttpClientCounters &counters, ESB::Allocator &allocator)
    : HttpMultiplexer(maxSockets, allocator),
      _connections(connections),
      _seedTransactionHandler(seedTransactionHandler),
      _socketFactory(*this, _factoryAllocator),
      _transactionFactory(_factoryAllocator),
      _ioBufferPoolAllocator(HttpConfig::Instance().ioBufferChunkSize(),
                             ESB::SystemConfig::Instance().cacheLineSize()),
      _ioBufferPool(HttpConfig::Instance().ioBufferSize() -
                    ESB_BUFFER_OVERHEAD),
      _clientStack(_epollMultiplexer, _socketFactory, _transactionFactory,
                   handler, counters, _ioBufferPool) {
  _socketFactory.setClientStack(_clientStack);
}

HttpClientMultiplexer::~HttpClientMultiplexer() {}

bool HttpClientMultiplexer::run(ESB::SharedInt *isRunning) {
  // TODO replace this loop with ESB::EventSocket and ESB::SharedQueue dispatch.
  for (ESB::UInt32 i = 0; i < _connections; ++i) {
    HttpClientTransaction *transaction = _transactionFactory.create();
    if (!transaction) {
      ESB_LOG_CRITICAL("Cannot allocate seed transaction");
      return false;
    }

    ESB::Error error = _seedTransactionHandler.modifyTransaction(transaction);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot modify seed transaction");
      _transactionFactory.release(transaction);
      return false;
    }

    error = _socketFactory.executeClientTransaction(transaction);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot execute seed transaction");
      _transactionFactory.release(transaction);
      return false;
    }
  }

  return _epollMultiplexer.run(isRunning);
}

const char *HttpClientMultiplexer::name() const {
  return _epollMultiplexer.name();
}

ESB::CleanupHandler *HttpClientMultiplexer::cleanupHandler() { return NULL; }

bool HttpClientMultiplexer::HttpClientStackImpl::isRunning() {
  return _multiplexer.isRunning();
}

HttpClientTransaction *
HttpClientMultiplexer::HttpClientStackImpl::createTransaction() {
  return _transactionFactory.create();
}

ESB::Error HttpClientMultiplexer::HttpClientStackImpl::executeClientTransaction(
    HttpClientTransaction *transaction) {
  return _socketFactory.executeClientTransaction(transaction);
}

void HttpClientMultiplexer::HttpClientStackImpl::destroyTransaction(
    HttpClientTransaction *transaction) {
  return _transactionFactory.release(transaction);
}

HttpClientMultiplexer::HttpClientStackImpl::HttpClientStackImpl(
    ESB::EpollMultiplexer &multiplexer, HttpClientSocketFactory &socketFactory,
    HttpClientTransactionFactory &transactionFactory,
    HttpClientHandler &handler, HttpClientCounters &counters,
    ESB::BufferPool &bufferPool)
    : _multiplexer(multiplexer),
      _socketFactory(socketFactory),
      _transactionFactory(transactionFactory),
      _handler(handler),
      _counters(counters),
      _bufferPool(bufferPool) {}

HttpClientMultiplexer::HttpClientStackImpl::~HttpClientStackImpl() {}

HttpClientHandler &HttpClientMultiplexer::HttpClientStackImpl::handler() {
  return _handler;
}

HttpClientCounters &HttpClientMultiplexer::HttpClientStackImpl::counters() {
  return _counters;
}

ESB::BufferPool &HttpClientMultiplexer::HttpClientStackImpl::bufferPool() {
  return _bufferPool;
}

}  // namespace ES
