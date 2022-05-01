#ifndef ES_HTTP_FILTER_H
#include <ESHttpFilter.h>
#endif

namespace ES {

HttpFilter::HttpFilter() {}

HttpFilter::~HttpFilter() {}

ESB::CleanupHandler *HttpFilter::cleanupHandler() { return NULL; }

}  // namespace ES
