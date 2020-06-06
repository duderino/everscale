#ifndef ES_HTTP_PROXY_H
#define ES_HTTP_PROXY_H

#ifndef ES_HTTP_SERVER_H
#include <ESHttpServer.h>
#endif

#ifndef ES_HTTP_PROXY_HANDLER_H
#include <ESHttpProxyHandler.h>
#endif

#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_COMMAND_H
#include <ESHttpClientCommand.h>
#endif

namespace ES {

class HttpProxy : public HttpServer {
 public:
  /**
   * Create a client stack.
   */
  HttpProxy(ESB::UInt32 threads, HttpProxyHandler &proxyHandler,
            ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpProxy();

  /**
   * Enqueue a command to be run on a multiplexer thread.  If the
   * command has a cleanup handler, the multiplexer will call its cleanup
   * handler after the command finishes.
   *
   * @param command The command to execute
   * @param idx the index of a specific multiplexer ranging from 0 to
   * threads()-1 inclusive.  If -1 then a random multiplexer will be picked.
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error push(HttpClientCommand *command, int idx = -1);

  inline const HttpClientCounters &clientCounters() const { return _clientCounters; }

 protected:
  virtual ESB::SocketMultiplexer *createMultiplexer();

 private:
  // disabled
  HttpProxy(const HttpProxy &);
  void operator=(const HttpProxy &);

  HttpProxyHandler &_proxyHandler;
  HttpClientHistoricalCounters _clientCounters;
};

}  // namespace ES

#endif
