#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#include <ESHttpEchoClientContext.h>
#endif

namespace ES {

volatile int HttpEchoClientContext::_TotalIterations = 0;
ESB::SharedInt HttpEchoClientContext::_RemainingIterations;
ESB::SharedInt HttpEchoClientContext::_CompletedIterations;

HttpEchoClientContext::HttpEchoClientContext(
    ESB::CleanupHandler &cleanupHandler)
    : _bytesSent(0U), _cleanupHandler(cleanupHandler) {}

HttpEchoClientContext::~HttpEchoClientContext() {}

}  // namespace ES
