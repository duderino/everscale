#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#include <ESHttpEchoClientContext.h>
#endif

namespace ES {
HttpEchoClientContext::HttpEchoClientContext(unsigned int remainingIterations, ESB::CleanupHandler &cleanupHandler)
    : _bytesSent(0U), _iterations(remainingIterations), _cleanupHandler(cleanupHandler) {}

HttpEchoClientContext::~HttpEchoClientContext() {}

}  // namespace ES
