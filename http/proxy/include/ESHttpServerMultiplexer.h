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

namespace ES {

class HttpServerMultiplexer : public HttpMultiplexer {
 public:
  HttpServerMultiplexer(ESB::UInt32 maxSockets,
                        ESB::ListeningTCPSocket &listeningSocket,
                        HttpServerHandler &serverHandler,
                        HttpServerCounters &serverCounters);

  virtual ~HttpServerMultiplexer();

  virtual const char *name() const;
  virtual bool run(ESB::SharedInt *isRunning);
  virtual ESB::CleanupHandler *cleanupHandler();

 private:
  // disabled
  HttpServerMultiplexer(const HttpServerMultiplexer &);
  void operator=(const HttpServerMultiplexer &);

  // This is what the server multiplexer exposes to server sockets
  class HttpServerStackImpl : public HttpServerStack {
   public:
    HttpServerStackImpl(ESB::EpollMultiplexer &multiplexer,
                        ESB::BufferPool &bufferPool,
                        HttpServerCounters &counters,
                        HttpServerTransactionFactory &transactionFactory,
                        HttpServerSocketFactory &socketFactory);
    virtual ~HttpServerStackImpl();
    virtual bool isRunning();
    virtual HttpServerTransaction *createTransaction();
    virtual void destroyTransaction(HttpServerTransaction *transaction);
    virtual ESB::Buffer *acquireBuffer();
    virtual void releaseBuffer(ESB::Buffer *buffer);
    virtual ESB::Error addServerSocket(ESB::TCPSocket::State &state);
    virtual HttpServerCounters &counters();

   private:
    // Disabled
    HttpServerStackImpl(const HttpServerStackImpl &);
    HttpServerStackImpl &operator=(const HttpServerStackImpl &);

    ESB::EpollMultiplexer &_multiplexer;
    ESB::BufferPool &_bufferPool;
    HttpServerCounters &_counters;
    HttpServerTransactionFactory &_transactionFactory;
    HttpServerSocketFactory &_socketFactory;
  };

  HttpServerSocketFactory _serverSocketFactory;
  HttpServerTransactionFactory _serverTransactionFactory;
  HttpServerStackImpl _serverStack;
  HttpListeningSocket _listeningSocket;
};

}  // namespace ES

#endif
