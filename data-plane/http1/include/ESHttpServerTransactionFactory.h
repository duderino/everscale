#ifndef ES_HTTP_SERVER_TRANSACTION_FACTORY_H
#define ES_HTTP_SERVER_TRANSACTION_FACTORY_H

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

#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

namespace ES {

/** A factory that creates and reuses HttpServerTransactions
 */
class HttpServerTransactionFactory {
 public:
  HttpServerTransactionFactory(ESB::Allocator &allocator);

  virtual ~HttpServerTransactionFactory();

  HttpServerTransaction *create();

  void release(HttpServerTransaction *transaction);

 private:
  class CleanupHandler : public ESB::CleanupHandler {
   public:
    /** Constructor
     */
    CleanupHandler(HttpServerTransactionFactory &factory);

    /** Destructor
     */
    virtual ~CleanupHandler();

    /** Destroy an object
     *
     * @param object The object to destroy
     */
    virtual void destroy(ESB::Object *object);

   private:
    HttpServerTransactionFactory &_factory;

    ESB_DISABLE_AUTO_COPY(CleanupHandler);
  };

  ESB::Allocator &_allocator;
  ESB::EmbeddedList _embeddedList;
  CleanupHandler _cleanupHandler;

  ESB_DEFAULT_FUNCS(HttpServerTransactionFactory);
};

}  // namespace ES

#endif
