#ifndef ES_HTTP_SERVER_TRANSACTION_FACTORY_H
#include <ESHttpServerTransactionFactory.h>
#endif

namespace ES {

HttpServerTransactionFactory::HttpServerTransactionFactory(
    ESB::Allocator &allocator)
    : _allocator(allocator), _embeddedList(), _cleanupHandler(*this) {}

HttpServerTransactionFactory::~HttpServerTransactionFactory() {
  HttpServerTransaction *transaction =
      (HttpServerTransaction *)_embeddedList.removeFirst();

  while (transaction) {
    transaction->~HttpServerTransaction();
    transaction = (HttpServerTransaction *)_embeddedList.removeFirst();
  }
}

HttpServerTransaction *HttpServerTransactionFactory::create() {
  HttpServerTransaction *transaction =
      (HttpServerTransaction *)_embeddedList.removeLast();

  if (!transaction) {
    transaction = new (_allocator) HttpServerTransaction(_cleanupHandler);
  }

  return transaction;
}

void HttpServerTransactionFactory::release(HttpServerTransaction *transaction) {
  if (!transaction) {
    return;
  }

  transaction->reset();
  _embeddedList.addLast(transaction);
}

HttpServerTransactionFactory::CleanupHandler::CleanupHandler(
    HttpServerTransactionFactory &factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpServerTransactionFactory::CleanupHandler::~CleanupHandler() {}

void HttpServerTransactionFactory::CleanupHandler::destroy(
    ESB::Object *object) {
  _factory.release((HttpServerTransaction *)object);
}

}  // namespace ES
