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

#ifndef ESB_CLIENT_TLS_CONTEXT_INDEX_H
#include <ESBClientTLSContextIndex.h>
#endif

namespace ES {

class HttpProxy : public HttpServer {
 public:
  /**
   * Create a proxy stack.
   */
  HttpProxy(const char *namePrefix, ESB::UInt32 threads, ESB::UInt32 idleTimeoutMsec, HttpProxyHandler &proxyHandler,
            ESB::Allocator &allocator = ESB::SystemAllocator::Instance());

  virtual ~HttpProxy();

  inline ESB::ClientTLSContextIndex &clientTlsContextIndex() { return _clientContextIndex; }

  inline const ESB::ClientTLSContextIndex &clientTlsContextIndex() const { return _clientContextIndex; }

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
  HttpProxyHandler &_proxyHandler;
  ESB::ClientTLSContextIndex _clientContextIndex;
  HttpClientHistoricalCounters _clientCounters;

  ESB_DEFAULT_FUNCS(HttpProxy);
};

}  // namespace ES

#endif
