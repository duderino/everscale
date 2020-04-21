#ifndef ES_HTTP_SERVER_MULTIPLEXER_H
#define ES_HTTP_SERVER_MULTIPLEXER_H

#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
#endif

#ifndef ES_HTTP_LISTENING_SOCKET_H
#include <ESHttpListeningSocket.h>
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

#ifndef ES_HTTP_SERVER_STACK_H
#include <ESHttpServerStack.h>
#endif

#ifndef ES_HTTP_SERVER_TRANSACTION_FACTORY_H
#include <ESHttpServerTransactionFactory.h>
#endif

#ifndef ES_HTTP_SERVER_COMMAND_SOCKET_H
#include <ESHttpServerCommandSocket.h>
#endif

namespace ES {

class HttpServerMultiplexer : public HttpMultiplexer {
 public:
  HttpServerMultiplexer(ESB::UInt32 maxSockets,
                        HttpServerHandler &serverHandler,
                        HttpServerCounters &serverCounters);

  virtual ~HttpServerMultiplexer();

  virtual const char *name() const;
  virtual bool run(ESB::SharedInt *isRunning);
  virtual ESB::CleanupHandler *cleanupHandler();

  /**
   * Enqueue a command in the multiplexer and wake it up.  When the multiplexer
   * wakes up, it will dequeue the command and execute it in it's thread of
   * control.
   *
   * @param command The command to execute
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  inline ESB::Error pushServerCommand(HttpServerCommand *command) {
    return _serverCommandSocket.push(command);
  }

 private:
  // disabled
  HttpServerMultiplexer(const HttpServerMultiplexer &);
  void operator=(const HttpServerMultiplexer &);

  // This is what the HttpServerMultiplexer exposes to HttpServerHandler
  class HttpServerStackImpl : public HttpServerStack {
   public:
    HttpServerStackImpl(ESB::Allocator &allocator,
                        ESB::EpollMultiplexer &multiplexer,
                        ESB::BufferPool &bufferPool, HttpServerHandler &handler,
                        HttpServerCounters &counters,
                        HttpServerTransactionFactory &transactionFactory,
                        HttpServerSocketFactory &socketFactory);
    virtual ~HttpServerStackImpl();
    virtual bool isRunning();
    virtual HttpServerTransaction *createServerTransaction();
    virtual void destroyTransaction(HttpServerTransaction *transaction);
    virtual ESB::Buffer *acquireBuffer();
    virtual void releaseBuffer(ESB::Buffer *buffer);
    virtual ESB::Error addServerSocket(ESB::TCPSocket::State &state);
    virtual ESB::Error addListeningSocket(ESB::ListeningTCPSocket &socket);
    virtual HttpServerCounters &serverCounters();

   private:
    // Disabled
    HttpServerStackImpl(const HttpServerStackImpl &);
    HttpServerStackImpl &operator=(const HttpServerStackImpl &);

    ESB::Allocator &_allocator;
    ESB::EpollMultiplexer &_multiplexer;
    ESB::BufferPool &_bufferPool;
    HttpServerHandler &_handler;
    HttpServerCounters &_counters;
    HttpServerTransactionFactory &_transactionFactory;
    HttpServerSocketFactory &_socketFactory;
  };

  HttpServerSocketFactory _serverSocketFactory;
  HttpServerTransactionFactory _serverTransactionFactory;
  HttpServerStackImpl _serverStack;

 protected:
  HttpServerCommandSocket _serverCommandSocket;
};

}  // namespace ES

#endif
