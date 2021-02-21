#ifndef ES_HTTP_HEADER_H
#include <ESHttpHeader.h>
#endif

namespace ES {

HttpHeader::HttpHeader(const char *fieldName, const char *fieldValue)
    : ESB::EmbeddedListElement(),
      _fieldName((const unsigned char *)fieldName),
      _fieldValue((const unsigned char *)fieldValue) {}

HttpHeader::HttpHeader(const unsigned char *fieldName, const unsigned char *fieldValue)
    : ESB::EmbeddedListElement(), _fieldName(fieldName), _fieldValue(fieldValue) {}

HttpHeader::~HttpHeader() {}

ESB::CleanupHandler *HttpHeader::cleanupHandler() { return NULL; }

}  // namespace ES
