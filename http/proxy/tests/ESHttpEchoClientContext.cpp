#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#include <ESHttpEchoClientContext.h>
#endif

namespace ES {
HttpEchoClientContext::HttpEchoClientContext(unsigned int remainingIterations)
    : _bytesSent(0U), _iterations(remainingIterations) {}

HttpEchoClientContext::~HttpEchoClientContext() {}
}  // namespace ES
