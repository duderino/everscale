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

namespace ES {

class HttpServerMultiplexer : public HttpMultiplexer {
 public:
  HttpServerMultiplexer(ESB::UInt32 maxSockets,
                        ESB::ListeningTCPSocket &listeningSocket,
                        HttpServerHandler &serverHandler,
                        HttpServerCounters &serverCounters,
                        ESB::Allocator &allocator);

  virtual ~HttpServerMultiplexer();

  virtual const char *name() const;
  virtual bool run(ESB::SharedInt *isRunning);
  virtual ESB::CleanupHandler *cleanupHandler();
  virtual ESB::Error initialize();
  virtual void destroy();

 private:
  // disabled
  HttpServerMultiplexer(const HttpServerMultiplexer &);
  void operator=(const HttpServerMultiplexer &);

  HttpServerSocketFactory _serverSocketFactory;
  HttpListeningSocket _listeningSocket;
};

}  // namespace ES

#endif
