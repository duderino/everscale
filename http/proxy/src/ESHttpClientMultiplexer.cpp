#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#include <ESHttpClientMultiplexer.h>
#endif

namespace ES {

HttpClientMultiplexer::HttpClientMultiplexer(
    ESB::UInt32 connections, HttpSeedTransactionHandler &seedTransactionHandler,
    ESB::UInt32 maxSockets, HttpClientHandler &clientHandler,
    HttpClientCounters &clientCounters, ESB::Allocator &allocator)
    : HttpMultiplexer(maxSockets, allocator),
      _connections(connections),
      _seedTransactionHandler(seedTransactionHandler),
      // TODO why pass client handler to both factories????
      _clientSocketFactory(*this, clientHandler, clientCounters,
                           _factoryAllocator),
      _clientTransactionFactory(clientHandler, _factoryAllocator) {}

HttpClientMultiplexer::~HttpClientMultiplexer() {}

ESB::Error HttpClientMultiplexer::initialize() {
  return _epollMultiplexer.initialize();
}

bool HttpClientMultiplexer::run(ESB::SharedInt *isRunning) {
  // TODO replace this loop with ESB::EventSocket and ESB::SharedQueue dispatch.
  for (ESB::UInt32 i = 0; i < _connections; ++i) {
    HttpClientTransaction *transaction = _clientTransactionFactory.create();
    if (!transaction) {
      ESB_LOG_CRITICAL("Cannot allocate seed transaction");
      return false;
    }

    ESB::Error error = _seedTransactionHandler.modifyTransaction(transaction);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot modify seed transaction");
      _clientTransactionFactory.release(transaction);
      return false;
    }

    error = _clientSocketFactory.executeClientTransaction(transaction);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "Cannot execute seed transaction");
      _clientTransactionFactory.release(transaction);
      return false;
    }
  }

  return _epollMultiplexer.run(isRunning);
}

void HttpClientMultiplexer::destroy() { _epollMultiplexer.destroy(); }

const char *HttpClientMultiplexer::name() const {
  return _epollMultiplexer.name();
}

ESB::CleanupHandler *HttpClientMultiplexer::cleanupHandler() { return NULL; }

}  // namespace ES
