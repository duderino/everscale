#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#include <ESHttpClientMultiplexer.h>
#endif

namespace ES {

HttpClientMultiplexer::HttpClientMultiplexer(
    ESB::UInt32 connections, HttpSeedTransactionHandler &seedTransactionHandler,
    ESB::UInt32 maxSockets, HttpClientHandler &clientHandler,
    HttpClientCounters &clientCounters)
    : HttpMultiplexer(maxSockets),
      _connections(connections),
      _seedTransactionHandler(seedTransactionHandler),
      _clientSocketFactory(*this, clientHandler, clientCounters,
                           _factoryAllocator),
      _clientTransactionFactory(_factoryAllocator),
      _clientStack(_epollMultiplexer, _clientSocketFactory,
                   _clientTransactionFactory) {
  _clientSocketFactory.setClientStack(_clientStack);
}

HttpClientMultiplexer::~HttpClientMultiplexer() {}

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

const char *HttpClientMultiplexer::name() const {
  return _epollMultiplexer.name();
}

ESB::CleanupHandler *HttpClientMultiplexer::cleanupHandler() { return NULL; }

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

HttpClientMultiplexer::HttpClientStackImpl::HttpClientStackImpl(
    ESB::EpollMultiplexer &multiplexer,
    HttpClientSocketFactory &clientSocketFactory,
    HttpClientTransactionFactory &clientTransactionFactory)
    : _multiplexer(multiplexer),
      _clientSocketFactory(clientSocketFactory),
      _clientTransactionFactory(clientTransactionFactory) {}

HttpClientMultiplexer::HttpClientStackImpl::~HttpClientStackImpl() {}

}  // namespace ES
