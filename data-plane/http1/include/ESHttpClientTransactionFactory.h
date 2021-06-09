#ifndef ES_HTTP_CLIENT_TRANSACTION_FACTORY_H
#define ES_HTTP_CLIENT_TRANSACTION_FACTORY_H

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

namespace ES {

/** A factory that creates and reuses HttpClientTransactions
 */
class HttpClientTransactionFactory {
 public:
  HttpClientTransactionFactory(ESB::Allocator &allocator);

  virtual ~HttpClientTransactionFactory();

  HttpClientTransaction *create();

  void release(HttpClientTransaction *transaction);

 private:
  class CleanupHandler : public ESB::CleanupHandler {
   public:
    /** Constructor
     */
    CleanupHandler(HttpClientTransactionFactory &factory);

    /** Destructor
     */
    virtual ~CleanupHandler();

    /** Destroy an object
     *
     * @param object The object to destroy
     */
    virtual void destroy(ESB::Object *object);

   private:
    HttpClientTransactionFactory &_factory;

    ESB_DISABLE_AUTO_COPY(CleanupHandler);
  };

  ESB::Allocator &_allocator;
  ESB::EmbeddedList _embeddedList;
  CleanupHandler _cleanupHandler;

  ESB_DEFAULT_FUNCS(HttpClientTransactionFactory);
};

}  // namespace ES

#endif
