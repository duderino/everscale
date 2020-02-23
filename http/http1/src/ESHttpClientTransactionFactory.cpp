#ifndef ES_HTTP_CLIENT_TRANSACTION_FACTORY_H
#include <ESHttpClientTransactionFactory.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ES {

HttpClientTransactionFactory::HttpClientTransactionFactory()
    : _allocator(), _embeddedList(), _mutex(), _cleanupHandler(this) {}

ESB::Error HttpClientTransactionFactory::initialize() {
  return _allocator.initialize(
      ESB_WORD_ALIGN(sizeof(HttpClientTransaction)) * 1000,
      ESB::SystemAllocator::GetInstance());
}

void HttpClientTransactionFactory::destroy() {
  HttpClientTransaction *transaction =
      (HttpClientTransaction *)_embeddedList.removeFirst();

  while (transaction) {
    transaction->~HttpClientTransaction();

    transaction = (HttpClientTransaction *)_embeddedList.removeFirst();
  }

  _allocator.destroy();
}

HttpClientTransactionFactory::~HttpClientTransactionFactory() {}

HttpClientTransaction *HttpClientTransactionFactory::create(
    HttpClientHandler *clientHandler) {
  HttpClientTransaction *transaction = 0;

  _mutex.writeAcquire();

  transaction = (HttpClientTransaction *)_embeddedList.removeLast();

  _mutex.writeRelease();

  if (0 == transaction) {
    transaction = new (&_allocator)
        HttpClientTransaction(clientHandler, &_cleanupHandler);

    if (0 == transaction) {
      return 0;
    }
  }

  transaction->setHandler(clientHandler);

  return transaction;
}

void HttpClientTransactionFactory::release(HttpClientTransaction *transaction) {
  if (!transaction) {
    return;
  }

  transaction->reset();

  _mutex.writeAcquire();

  _embeddedList.addLast(transaction);

  _mutex.writeRelease();
}

HttpClientTransactionFactory::CleanupHandler::CleanupHandler(
    HttpClientTransactionFactory *factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpClientTransactionFactory::CleanupHandler::~CleanupHandler() {}

void HttpClientTransactionFactory::CleanupHandler::destroy(
    ESB::Object *object) {
  _factory->release((HttpClientTransaction *)object);
}

}  // namespace ES
