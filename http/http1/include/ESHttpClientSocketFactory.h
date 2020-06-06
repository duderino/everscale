#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#define ES_HTTP_CLIENT_SOCKET_FACTORY_H

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_H
#include <ESHttpClientSocket.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_SHARED_EMBEDDED_MAP_H
#include <ESBSharedEmbeddedMap.h>
#endif

namespace ES {

/** A factory that creates and reuses HttpClientSockets
 */
class HttpClientSocketFactory {
 public:
  /** Constructor
   *
   */
  HttpClientSocketFactory(HttpMultiplexerExtended &multiplexer, HttpClientHandler &handler,
                          HttpClientCounters &counters, ESB::Allocator &allocator);

  /** Destructor.
   */
  virtual ~HttpClientSocketFactory();

  HttpClientSocket *create(HttpClientTransaction *transaction);

  /**
   * Execute the client transaction.  If this method returns ESB_SUCCESS, then
   * the transaction will be cleaned up automatically after it finishes.  If
   * this method returns anything else then the caller should clean it up with
   * destroyClientTransaction
   *
   * @param transaction The transaction
   * @return ESB_SUCCESS if the transaction was successfully started, another
   * error code otherwise.  If error, cleanup the transaction with the
   * destroyClientTransaction method.
   */
  ESB::Error executeClientTransaction(HttpClientTransaction *transaction);

  void release(HttpClientSocket *socket);

 private:
  // Disabled
  HttpClientSocketFactory(const HttpClientSocketFactory &);
  HttpClientSocketFactory &operator=(const HttpClientSocketFactory &);

  // To cleanup client sockets created by this factory.  Cleanup returns the
  // socket to the factory for subsequent reuse.
  class CleanupHandler : public ESB::CleanupHandler {
   public:
    /** Constructor
     */
    CleanupHandler(HttpClientSocketFactory &factory);

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

    HttpClientSocketFactory &_factory;
  };

  // Callbacks for the hash table-based connection pool
  class SocketAddressCallbacks : public ESB::SharedEmbeddedMap::Callbacks {
   public:
    SocketAddressCallbacks(ESB::Allocator &allocator);

    virtual int compare(const void *f, const void *s) const;
    virtual ESB::UInt32 hash(const void *key) const;
    virtual void cleanup(ESB::EmbeddedMapElement *element);

   private:
    // Disabled
    SocketAddressCallbacks(const SocketAddressCallbacks &);
    SocketAddressCallbacks &operator=(const SocketAddressCallbacks &);

    ESB::Allocator &_allocator;
  };

  HttpMultiplexerExtended &_multiplexer;
  HttpClientHandler &_handler;
  HttpClientCounters &_counters;
  ESB::Allocator &_allocator;
  SocketAddressCallbacks _callbacks;
  ESB::SharedEmbeddedMap _map;
  ESB::EmbeddedList _sockets;
  CleanupHandler _cleanupHandler;
};

}  // namespace ES

#endif
