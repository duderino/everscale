#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#define ES_HTTP_CLIENT_SOCKET_FACTORY_H

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_MAP_H
#include <ESBMap.h>
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

#ifndef ESB_SYSTEM_DNS_CLIENT_H
#include <ESBSystemDnsClient.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_H
#include <ESBSocketMultiplexer.h>
#endif

namespace ES {

/** A factory that creates and reuses HttpClientSockets
 */
class HttpClientSocketFactory : public HttpClientSocket::RetryHandler {
 public:
  /** Constructor
   *
   */
  HttpClientSocketFactory(ESB::SocketMultiplexer &multiplexer,
                          HttpClientHandler &handler,
                          HttpClientCounters &counters,
                          ESB::Allocator &allocator);

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

  virtual ESB::Error retry(HttpClientTransaction *transaction);

 private:
  // Disabled
  HttpClientSocketFactory(const HttpClientSocketFactory &);
  HttpClientSocketFactory &operator=(const HttpClientSocketFactory &);

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

  ESB::SocketMultiplexer &_multiplexer;
  HttpClientHandler &_handler;
  HttpClientCounters &_counters;
  ESB::Allocator &_allocator;
  ESB::Map _map;
  ESB::EmbeddedList _sockets;
  CleanupHandler _cleanupHandler;
  ESB::SystemDnsClient _dnsClient;
};

}  // namespace ES

#endif
