#ifndef ES_HTTP_PROXY_CONTEXT_H
#include <ESHttpRoutingProxyContext.h>
#endif

namespace ES {
HttpRoutingProxyContext::HttpRoutingProxyContext()
    : _state(State::SERVER_REQUEST_WAIT), _serverStream(NULL), _clientStream(NULL) {}
HttpRoutingProxyContext::~HttpRoutingProxyContext() {}
}  // namespace ES
