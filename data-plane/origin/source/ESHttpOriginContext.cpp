#ifndef ES_HTTP_ORIGIN_CONTEXT_H
#include <ESHttpOriginContext.h>
#endif

namespace ES {
HttpOriginContext::HttpOriginContext() : _bytesSent(0U), _bytesReceived(0U) {}
HttpOriginContext::~HttpOriginContext() {}
}  // namespace ES
