#ifndef ES_HTTP_PROXY_MULTIPLEXER_H
#define ES_HTTP_PROXY_MULTIPLEXER_H

#ifndef ES_HTTP_SERVER_MULTIPLEXER_H
#include <ESHttpServerMultiplexer.h>
#endif

#ifndef ES_HTTP_CLIENT_MULTIPLEXER_H
#include <ESHttpClientMultiplexer.h>
#endif

#ifndef ES_HTTP_PROXY_HANDLER_H
#include <ESHttpProxyHandler.h>
#endif

namespace ES {

class HttpProxyMultiplexer : public HttpServerMultiplexer {
 public:
  HttpProxyMultiplexer(ESB::UInt32 maxSockets, HttpProxyHandler &proxyHandler,
                       HttpClientCounters &clientCounters,
                       HttpServerCounters &serverCounters);

  virtual ~HttpProxyMultiplexer();

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
  inline ESB::Error pushClientCommand(HttpClientCommand *command) {
    return _clientCommandSocket.push(command);
  }

 private:
  // disabled
  HttpProxyMultiplexer(const HttpProxyMultiplexer &);
  void operator=(const HttpProxyMultiplexer &);

  HttpClientSocketFactory _clientSocketFactory;
  HttpClientTransactionFactory _clientTransactionFactory;
  HttpClientMultiplexer::HttpClientStackImpl _clientStack;
  HttpClientCommandSocket _clientCommandSocket;
};

}  // namespace ES

#endif
