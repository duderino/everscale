#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#define ES_HTTP_CLIENT_MULTIPLEXER_H

#ifndef ES_HTTP_MULTIPLEXER_H
#include <ESHttpMultiplexer.h>
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

class HttpClientMultiplexer : public HttpMultiplexer {
 public:
  HttpClientMultiplexer(ESB::UInt32 connections,
                        HttpSeedTransactionHandler &seedTransactionHandler,
                        ESB::UInt32 maxSockets,
                        HttpClientHandler &clientHandler,
                        HttpClientCounters &clientCounters,
                        ESB::Allocator &allocator);

  virtual ~HttpClientMultiplexer();

  virtual const char *name() const;
  virtual bool run(ESB::SharedInt *isRunning);
  virtual ESB::CleanupHandler *cleanupHandler();
  virtual ESB::Error initialize();
  virtual void destroy();

 private:
  // disabled
  HttpClientMultiplexer(const HttpClientMultiplexer &);
  void operator=(const HttpClientMultiplexer &);

  ESB::UInt32 _connections;
  HttpSeedTransactionHandler &_seedTransactionHandler;
  HttpClientSocketFactory _clientSocketFactory;
  HttpClientTransactionFactory _clientTransactionFactory;
};

}  // namespace ES

#endif
