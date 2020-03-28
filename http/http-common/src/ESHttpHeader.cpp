#ifndef ES_HTTP_HEADER_H
#include <ESHttpHeader.h>
#endif

namespace ES {

HttpHeader::HttpHeader(const unsigned char *fieldName,
                       const unsigned char *fieldValue)
    : ESB::EmbeddedListElement(),
      _fieldName(fieldName),
      _fieldValue(fieldValue) {}

HttpHeader::~HttpHeader() {}

ESB::CleanupHandler *HttpHeader::cleanupHandler() { return 0; }
}  // namespace ES
