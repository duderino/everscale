#ifndef ES_HTTP_PROXY_CONTEXT_H
#include <ESHttpRoutingProxyContext.h>
#endif

namespace ES {

#define ESB_PROXY_RECEIVED_OUTBOUND_RESPONSE (1 << 0)

HttpRoutingProxyContext::HttpRoutingProxyContext()
    : _serverStream(NULL),
      _clientStream(NULL),
      _flags(0),
      _clientStreamResponseOffset(0U),
      _serverStreamRequestOffset(0U),
      _requestBodyBytesForwarded(0U),
      _responseBodyBytesForwarded(0U) {}

HttpRoutingProxyContext::~HttpRoutingProxyContext() {
  assert(!_serverStream);
  assert(!_clientStream);
}

bool HttpRoutingProxyContext::receivedOutboundResponse() const { return _flags & ESB_PROXY_RECEIVED_OUTBOUND_RESPONSE; }

void HttpRoutingProxyContext::setReceivedOutboundResponse(bool receivedOutboundResponse) {
  if (receivedOutboundResponse) {
    _flags |= ESB_PROXY_RECEIVED_OUTBOUND_RESPONSE;
  } else {
    _flags &= ~ESB_PROXY_RECEIVED_OUTBOUND_RESPONSE;
  }
}

}  // namespace ES
