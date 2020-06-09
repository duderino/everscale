#ifndef ES_HTTP_PROXY_CONTEXT_H
#include <ESHttpRoutingProxyContext.h>
#endif

namespace ES {

HttpRoutingProxyContext::HttpRoutingProxyContext()
    : _serverStream(NULL), _clientStream(NULL), _clientStreamResponseOffset(0U), _serverStreamRequestOffset(0U) {}

HttpRoutingProxyContext::~HttpRoutingProxyContext() {}

}  // namespace ES
