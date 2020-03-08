#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#define ES_HTTP_CLIENT_SOCKET_FACTORY_H

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_CLEANUP_HANDLER_H
#include <ESBCleanupHandler.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ES_HTTP_CONNECTION_POOL_H
#include <ESHttpConnectionPool.h>
#endif

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

namespace ES {

/** A factory that creates and reuses HttpClientSockets
 */
class HttpClientSocketFactory {
 public:
  /** Constructor
   *
   * @param logger A logger
   */
  HttpClientSocketFactory(HttpClientCounters *clientCounters);

  /** Destructor.
   */
  virtual ~HttpClientSocketFactory();

  ESB::Error initialize();

  void destroy();

  HttpClientSocket *create(HttpConnectionPool *pool,
                           HttpClientTransaction *transaction);

  void release(HttpClientSocket *socket);

 private:
  // Disabled
  HttpClientSocketFactory(const HttpClientSocketFactory &);
  HttpClientSocketFactory &operator=(const HttpClientSocketFactory &);

  class CleanupHandler : public ESB::CleanupHandler {
   public:
    /** Constructor
     */
    CleanupHandler(HttpClientSocketFactory *factory);

    /** Destructor
     */
    virtual ~CleanupHandler();

    /** Destroy an object
     *
     * @param object The object to destroy
     */
    virtual void destroy(ESB::Object *object);

   private:
    // Disabled
    CleanupHandler(const CleanupHandler &);
    void operator=(const CleanupHandler &);

    HttpClientSocketFactory *_factory;
  };

  HttpClientCounters *_clientCounters;
  ESB::DiscardAllocator _unprotectedAllocator;
  ESB::SharedAllocator _allocator;
  ESB::Map _map;
  ESB::EmbeddedList _embeddedList;
  ESB::Mutex _mutex;
  CleanupHandler _cleanupHandler;
};

}  // namespace ES

#endif
