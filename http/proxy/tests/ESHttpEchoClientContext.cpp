#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#include <ESHttpEchoClientContext.h>
#endif

namespace ES {

ESB::SharedInt HttpEchoClientContext::_RemainingIterations;

HttpEchoClientContext::HttpEchoClientContext(
    ESB::CleanupHandler &cleanupHandler)
    : _bytesSent(0U), _cleanupHandler(cleanupHandler) {}

HttpEchoClientContext::~HttpEchoClientContext() {}

}  // namespace ES
