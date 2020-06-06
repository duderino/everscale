#ifndef ES_HTTP_ROUTER_H
#define ES_HTTP_ROUTER_H

#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

#ifndef ES_HTTP_SERVER_STREAM_H
#include <ESHttpServerStream.h>
#endif

#ifndef ESB_SOCKET_ADDRESS_H
#include <ESBSocketAddress.h>
#endif

namespace ES {

class HttpRouter {
 public:
  HttpRouter();

  virtual ~HttpRouter();

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
  virtual ESB::Error route(const HttpServerStream &serverStream,
                           HttpClientTransaction &clientTransaction,
                           ESB::SocketAddress &destination) = 0;

 private:
  // Disabled
  HttpRouter(const HttpRouter &);
  void operator=(const HttpRouter &);
};

}  // namespace ES

#endif
