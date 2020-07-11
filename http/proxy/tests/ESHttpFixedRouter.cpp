#ifndef ES_HTTP_FIXED_ROUTER_H
#include "ESHttpFixedRouter.h"
#endif

namespace ES {

HttpFixedRouter::HttpFixedRouter(ESB::SocketAddress destination) : _destination(destination) {}

HttpFixedRouter::~HttpFixedRouter() {}

ESB::Error HttpFixedRouter::route(const HttpServerStream &serverStream, HttpClientTransaction &clientTransaction,
                                  ESB::SocketAddress &destination) {
  destination = _destination;
  return ESB_SUCCESS;
}

}  // namespace ES
