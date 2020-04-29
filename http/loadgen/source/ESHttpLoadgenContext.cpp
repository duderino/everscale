#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#include <ESHttpLoadgenContext.h>
#endif

namespace ES {

volatile int HttpLoadgenContext::_TotalIterations = 0;
ESB::SharedInt HttpLoadgenContext::_RemainingIterations;
ESB::SharedInt HttpLoadgenContext::_CompletedIterations;

HttpLoadgenContext::HttpLoadgenContext(ESB::CleanupHandler &cleanupHandler)
    : _bytesSent(0U), _cleanupHandler(cleanupHandler) {}

HttpLoadgenContext::~HttpLoadgenContext() {}

}  // namespace ES
