#ifndef ES_HTTP_PROXY_MULTIPLEXER_H
#define ES_HTTP_PROXY_MULTIPLEXER_H

#ifndef ES_HTTP_MULTIPLEXER_EXTENDED_H
#include <ESHttpMultiplexerExtended.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_SOCKET_FACTORY_H
#include <ESHttpClientSocketFactory.h>
#endif

#ifndef ES_HTTP_CLIENT_TRANSACTION_FACTORY_H
#include <ESHttpClientTransactionFactory.h>
#endif

#ifndef ES_HTTP_CLIENT_COMMAND_SOCKET_H
#include <ESHttpClientCommandSocket.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_SERVER_COUNTERS_H
#include <ESHttpServerCounters.h>
#endif

#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#include <ESHttpServerSocketFactory.h>
#endif

#ifndef ES_HTTP_SERVER_TRANSACTION_FACTORY_H
#include <ESHttpServerTransactionFactory.h>
#endif

#ifndef ES_HTTP_SERVER_COMMAND_SOCKET_H
#include <ESHttpServerCommandSocket.h>
#endif

#ifndef ES_HTTP_LISTENING_SOCKET_H
#include <ESHttpListeningSocket.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_EPOLL_MULTIPLEXER_H
#include <ESBEpollMultiplexer.h>
#endif

#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

namespace ES {

class HttpProxyMultiplexer : public ESB::SocketMultiplexer, public HttpMultiplexerExtended {
 public:
  /**
   * Create a proxy-mode (client + server) multiplexer.
   *
   * @param maxSockets
   * @param clientHandler
   * @param serverHandler
   * @param clientCounters
   * @param serverCounters
   */
  HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                       HttpClientHandler &clientHandler, HttpServerHandler &serverHandler,
                       HttpClientCounters &clientCounters, HttpServerCounters &serverCounters);

  /**
   * Create a client-only multiplexer.
   *
   * @param maxSockets
   * @param clientHandler
   * @param clientCounters
   */
  HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                       HttpClientHandler &clientHandler, HttpClientCounters &clientCounters);

  /**
   * Create a server-only multiplexer
   *
   * @param maxSockets
   * @param serverHandler
   * @param serverCounters
   */
  HttpProxyMultiplexer(const char *namePrefix, ESB::UInt32 maxSockets, ESB::UInt32 idleTimeoutMsec,
                       HttpServerHandler &serverHandler, HttpServerCounters &serverCounters);

  virtual ~HttpProxyMultiplexer();

  /**
   * Enqueue a command in the multiplexer and wake it up.  When the multiplexer
   * wakes up, it will dequeue the command and execute it in it's thread of
   * control.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  inline ESB::Error pushClientCommand(HttpClientCommand *command) { return _clientCommandSocket.push(command); }

  /**
   * Enqueue a command in the multiplexer and wake it up.  When the multiplexer
   * wakes up, it will dequeue the command and execute it in it's thread of
   * control.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  inline ESB::Error pushServerCommand(HttpServerCommand *command) { return _serverCommandSocket.push(command); }

  //
  // ESB::Command
  //

  virtual const char *name() const;
  virtual bool run(ESB::SharedInt *isRunning);
  virtual ESB::CleanupHandler *cleanupHandler();

  //
  // ESB::SocketMultiplexer
  //

  virtual ESB::Error addMultiplexedSocket(ESB::MultiplexedSocket *multiplexedSocket);
  virtual ESB::Error updateMultiplexedSocket(ESB::MultiplexedSocket *socket);
  virtual ESB::Error removeMultiplexedSocket(ESB::MultiplexedSocket *socket);
  virtual int currentSockets() const;
  virtual int maximumSockets() const;
  virtual bool isRunning() const;

  //
  //
  // ES::HttpMultiplexer
  //

  virtual bool shutdown();
  virtual ESB::Buffer *acquireBuffer();
  virtual void releaseBuffer(ESB::Buffer *buffer);

  virtual HttpClientTransaction *createClientTransaction();
  virtual ESB::Error executeClientTransaction(HttpClientTransaction *transaction);
  virtual void destroyClientTransaction(HttpClientTransaction *transaction);

  virtual HttpServerTransaction *createServerTransaction();
  virtual void destroyServerTransaction(HttpServerTransaction *transaction);

  virtual ESB::Error addServerSocket(ESB::Socket::State &state);
  virtual ESB::Error addListeningSocket(ESB::ListeningSocket &socket);
  virtual HttpServerCounters &serverCounters();

  virtual ESB::SocketMultiplexer &multiplexer();

 private:
  // disabled
  HttpProxyMultiplexer(const HttpProxyMultiplexer &);
  void operator=(const HttpProxyMultiplexer &);

  ESB::DiscardAllocator _ioBufferPoolAllocator;
  ESB::BufferPool _ioBufferPool;
  ESB::DiscardAllocator _factoryAllocator;
  ESB::EpollMultiplexer _multiplexer;
  HttpServerSocketFactory _serverSocketFactory;
  HttpServerTransactionFactory _serverTransactionFactory;
  HttpServerCommandSocket _serverCommandSocket;
  HttpClientSocketFactory _clientSocketFactory;
  HttpClientTransactionFactory _clientTransactionFactory;
  HttpClientCommandSocket _clientCommandSocket;
  HttpClientHandler &_clientHandler;
  HttpServerHandler &_serverHandler;
  HttpClientCounters &_clientCounters;
  HttpServerCounters &_serverCounters;
};

}  // namespace ES

#endif
