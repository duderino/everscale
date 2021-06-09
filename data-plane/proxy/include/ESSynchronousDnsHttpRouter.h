#ifndef ES_SYNCHRONOUS_DNS_HTTP_ROUTER_H
#define ES_SYNCHRONOUS_DNS_HTTP_ROUTER_H

#ifndef ES_HTTP_ROUTER_H
#include <ESHttpRouter.h>
#endif

#ifndef ESB_SYSTEM_DNS_CLIENT_H
#include <ESBSystemDnsClient.h>
#endif

namespace ES {

class SynchronousDnsHttpRouter : public HttpRouter {
 public:
  SynchronousDnsHttpRouter();

  virtual ~SynchronousDnsHttpRouter();

  /**
   * Given an inbound stream with a populated server transaction, populate the
   * request headers in an empty outbound client transaction and set the
   * destination address for the outbound client request.
   *
   * @param serverStream A HttpStream with a populated inbound HttpRequest
   * @param clientTransaction A HttpClientTransaction with an outbound
   * HttpRequest to be populated by this implementation
   * @param destination An empty destination to be populated by this
   * implementation with the destination IP address
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual ESB::Error route(const HttpServerStream &serverStream, HttpClientTransaction &clientTransaction,
                           ESB::SocketAddress &destination);

 private:
  ESB::SystemDnsClient _dnsClient;

  ESB_DEFAULT_FUNCS(SynchronousDnsHttpRouter);
};

}  // namespace ES

#endif
