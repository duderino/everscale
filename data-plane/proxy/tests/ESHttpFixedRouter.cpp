#ifndef ES_HTTP_FIXED_ROUTER_H
#include "ESHttpFixedRouter.h"
#endif

namespace ES {

HttpFixedRouter::HttpFixedRouter(ESB::SocketAddress destination) : _destination(destination) {}

HttpFixedRouter::~HttpFixedRouter() {}

ESB::Error HttpFixedRouter::route(const HttpServerStream &serverStream, HttpClientTransaction &clientTransaction,
                                  ESB::SocketAddress &destination) {
#ifndef NDEBUG
  if (serverStream.secure()) {
    assert(ESB::SocketAddress::TLS == _destination.type());
  } else {
    assert(ESB::SocketAddress::TCP == _destination.type());
  }
#endif

  destination = _destination;
  return ESB_SUCCESS;
}

}  // namespace ES
