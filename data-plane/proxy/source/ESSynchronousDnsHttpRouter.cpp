#ifndef ES_SYNCHRONOUS_DNS_HTTP_ROUTER_H
#include <ESSynchronousDnsHttpRouter.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ES {

SynchronousDnsHttpRouter::SynchronousDnsHttpRouter() {}

SynchronousDnsHttpRouter::~SynchronousDnsHttpRouter() {}

ESB::Error SynchronousDnsHttpRouter::route(const HttpServerStream &serverStream,
                                           HttpClientTransaction &clientTransaction, ESB::SocketAddress &destination) {
  // TODO Make resolver async
  char hostname[ESB_MAX_HOSTNAME + 1];
  hostname[0] = 0;
  ESB::UInt16 port = 0;
  bool isSecure = false;

  ESB::Error error = clientTransaction.request().parsePeerAddress(hostname, sizeof(hostname), &port, &isSecure);

  if (ESB_SUCCESS != error) {
    ESB_LOG_DEBUG("Cannot extract hostname from request");
    return error;
  }

  return _dnsClient.resolve(destination, hostname, port, isSecure);
}

}  // namespace ES
