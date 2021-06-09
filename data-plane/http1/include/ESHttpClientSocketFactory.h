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

#ifndef ESB_CONNECTION_POOL_H
#include <ESBConnectionPool.h>
#endif

#ifndef ESB_CLIENT_TLS_CONTEXT_INDEX_H
#include <ESBClientTLSContextIndex.h>
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
                          HttpClientCounters &counters, ESB::ClientTLSContextIndex &contextIndex,
                          ESB::Allocator &allocator);

  /** Destructor.
   */
  virtual ~HttpClientSocketFactory();

  /**
   * Create a new http client socket, potentially reusing the underlying TCP connection in the process.
   *
   * @param transaction The client transction which will be executed on the socket
   * @param socket If successful, this will point to the created socket
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error create(HttpClientTransaction *transaction, HttpClientSocket **socket);

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

  /**
   * Return a socket to the factory.  If the socket is still connected, it will be reused.  If disconnected, the
   * memory will be reclaimed.
   *
   * @param socket The socket to release
   */
  void release(HttpClientSocket *socket);

 private:
  const char *name() const;

  // To cleanup client sockets created by this factory.  The CleanupHandler returns the
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
    HttpClientSocketFactory &_factory;

    ESB_DISABLE_AUTO_COPY(CleanupHandler);
  };

  HttpMultiplexerExtended &_multiplexer;
  HttpClientHandler &_handler;
  HttpClientCounters &_counters;
  ESB::Allocator &_allocator;
  ESB::ConnectionPool _connectionPool;
  ESB::EmbeddedList _deconstructedHttpSockets;
  CleanupHandler _cleanupHandler;

  ESB_DEFAULT_FUNCS(HttpClientSocketFactory);
};

}  // namespace ES

#endif
