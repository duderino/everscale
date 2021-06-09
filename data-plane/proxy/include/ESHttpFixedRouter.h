#ifndef ES_HTTP_FIXED_ROUTER_H
#define ES_HTTP_FIXED_ROUTER_H

#ifndef ES_HTTP_ROUTER_H
#include <ESHttpRouter.h>
#endif

namespace ES {

/**
 * This router blindly forwards all requests to a given address.
 */
class HttpFixedRouter : public HttpRouter {
 public:
  HttpFixedRouter(ESB::SocketAddress destination);
  virtual ~HttpFixedRouter();

  virtual ESB::Error route(const HttpServerStream &serverStream, HttpClientTransaction &clientTransaction,
                           ESB::SocketAddress &destination);

 private:
  ESB::SocketAddress _destination;

  ESB_DEFAULT_FUNCS(HttpFixedRouter);
};

}  // namespace ES

#endif
