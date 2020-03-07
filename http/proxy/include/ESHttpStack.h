#ifndef ES_HTTP_STACK_H
#define ES_HTTP_STACK_H

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESBEpollMultiplexerFactory.h>
#endif

#ifndef ESB_SOCKET_MULTIPLEXER_DISPATCHER_H
#include <ESBSocketMultiplexerDispatcher.h>
#endif

#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

#ifndef ESB_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESBAllocatorCleanupHandler.h>
#endif

#ifndef ES_HTTP_LISTENING_SOCKET_H
#include <ESHttpListeningSocket.h>
#endif

#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#include <ESHttpServerSocketFactory.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ES_HTTP_CONNECTION_POOL_H
#include <ESHttpConnectionPool.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_FACTORY_H
#include <ESHttpClientTransactionFactory.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

#ifndef ES_HTTP_CONNECTION_POOL_H
#include <ESHttpConnectionPool.h>
#endif

#ifndef ESB_DNS_CLIENT_H
#include <ESBDnsClient.h>
#endif

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

namespace ES {

class HttpStack : public HttpConnectionPool {
 public:
  /**
   * Create a client and server stack.
   */
  HttpStack(HttpServerHandler *serverHandler, ESB::DnsClient *dnsClient,
            int port, int threads, HttpClientCounters *clientCounters,
            HttpServerCounters *serverCounters, ESB::Logger *logger);

  /**
   * Create only a client stack.
   */
  HttpStack(ESB::DnsClient *dnsClient, int threads,
            HttpClientCounters *clientCounters, ESB::Logger *logger);

  virtual ~HttpStack();

  ESB::Error initialize();

  ESB::Error start();

  ESB::Error stop();

  void destroy();

  /**
   * Create a new client transaction
   *
   * @param handler The handler
   * @return a new client transaction if successful, null otherwise
   */
  virtual HttpClientTransaction *createClientTransaction(
      HttpClientHandler *clientHandler);

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
  virtual ESB::Error executeClientTransaction(
      HttpClientTransaction *transaction);

  /**
   * Cleanup the client transaction.  Note that this will not free any
   * app-specific context.  Call this only if executeClientTransaction doesn't
   * return ESB_SUCCESS
   *
   * @param transaction The transaction to cleanup.
   */
  virtual void destroyClientTransaction(HttpClientTransaction *transaction);

  inline HttpClientCounters *getClientCounters() { return _clientCounters; }

  inline const HttpClientCounters *getClientCounters() const {
    return _clientCounters;
  }

  inline HttpServerCounters *getServerCounters() { return _serverCounters; }

  inline const HttpServerCounters *getServerCounters() const {
    return _serverCounters;
  }

 private:
  // disabled
  HttpStack(const HttpStack &);
  void operator=(const HttpStack &);

  typedef enum {
    ES_HTTP_STACK_IS_INITIALIZED = 0,
    ES_HTTP_STACK_IS_STARTED = 1,
    ES_HTTP_STACK_IS_STOPPED = 2,
    ES_HTTP_STACK_IS_DESTROYED = 3
  } HttpStackState;

  int _port;
  int _threads;
  ESB::SharedInt _state;
  ESB::DnsClient *_dnsClient;
  ESB::Logger *_logger;
  HttpServerHandler *_serverHandler;
  HttpServerCounters *_serverCounters;
  HttpClientCounters *_clientCounters;
  ESB::DiscardAllocator _discardAllocator;
  ESB::SharedAllocator _rootAllocator;
  ESB::AllocatorCleanupHandler _rootAllocatorCleanupHandler;
  ESB::EpollMultiplexerFactory _epollFactory;
  ESB::ListeningTCPSocket _listeningSocket;
  HttpServerSocketFactory _serverSocketFactory;
  HttpClientSocketFactory _clientSocketFactory;
  HttpClientTransactionFactory _clientTransactionFactory;
  ESB::SocketMultiplexerDispatcher _dispatcher;
};

}  // namespace ES

#endif
