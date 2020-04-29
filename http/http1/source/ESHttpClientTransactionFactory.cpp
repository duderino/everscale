#ifndef ES_HTTP_CLIENT_TRANSACTION_FACTORY_H
#include <ESHttpClientTransactionFactory.h>
#endif

namespace ES {

HttpClientTransactionFactory::HttpClientTransactionFactory(
    ESB::Allocator &allocator)
    : _allocator(allocator), _embeddedList(), _cleanupHandler(*this) {}

HttpClientTransactionFactory::~HttpClientTransactionFactory() {
  while (true) {
    HttpClientTransaction *transaction =
        (HttpClientTransaction *)_embeddedList.removeFirst();
    if (!transaction) {
      break;
    }
    transaction->~HttpClientTransaction();
    _allocator.deallocate(transaction);
  }
}

HttpClientTransaction *HttpClientTransactionFactory::create() {
  HttpClientTransaction *transaction =
      (HttpClientTransaction *)_embeddedList.removeLast();

  if (!transaction) {
    transaction = new (_allocator) HttpClientTransaction(_cleanupHandler);

    if (!transaction) {
      return NULL;
    }
  }

  return transaction;
}

void HttpClientTransactionFactory::release(HttpClientTransaction *transaction) {
  if (!transaction) {
    return;
  }

  transaction->reset();
  _embeddedList.addLast(transaction);
}

HttpClientTransactionFactory::CleanupHandler::CleanupHandler(
    HttpClientTransactionFactory &factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpClientTransactionFactory::CleanupHandler::~CleanupHandler() {}

void HttpClientTransactionFactory::CleanupHandler::destroy(
    ESB::Object *object) {
  _factory.release((HttpClientTransaction *)object);
}

}  // namespace ES
