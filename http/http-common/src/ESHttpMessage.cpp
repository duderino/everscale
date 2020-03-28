#ifndef ES_HTTP_MESSAGE_H
#include <ESHttpMessage.h>
#endif

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

namespace ES {

HttpMessage::HttpMessage() : _flags(0x00), _version(110), _headers() {}

HttpMessage::~HttpMessage() {}

const HttpHeader *HttpMessage::getHeader(unsigned const char *fieldName) const {
  if (0 == fieldName) {
    return 0;
  }

  for (const HttpHeader *header = (const HttpHeader *)_headers.first(); header;
       header = (const HttpHeader *)header->next()) {
    if (0 == strcasecmp((const char *)fieldName,
                        (const char *)header->getFieldName())) {
      return header;
    }
  }

  return 0;
}

ESB::Error HttpMessage::addHeader(unsigned const char *fieldName,
                                  unsigned const char *fieldValue,
                                  ESB::Allocator *allocator) {
  if (!fieldName || !allocator) {
    return ESB_NULL_POINTER;
  }

  int fieldNameLength = strlen((const char *)fieldName);
  int fieldValueLength = strlen((const char *)fieldValue);

  unsigned char *block = (unsigned char *)allocator->allocate(
      sizeof(HttpHeader) + fieldNameLength + fieldValueLength + 2);

  if (!block) {
    return ESB_OUT_OF_MEMORY;
  }

  memcpy(block + sizeof(HttpHeader), fieldName, fieldNameLength);
  block[sizeof(HttpHeader) + fieldNameLength] = 0;

  memcpy(block + sizeof(HttpHeader) + fieldNameLength + 1, fieldValue,
         fieldValueLength);
  block[sizeof(HttpHeader) + fieldNameLength + fieldValueLength + 1] = 0;

  HttpHeader *header =
      new (block) HttpHeader(block + sizeof(HttpHeader),
                             block + sizeof(HttpHeader) + fieldNameLength + 1);

  _headers.addLast(header);

  return ESB_SUCCESS;
}

ESB::Error HttpMessage::addHeader(ESB::Allocator *allocator,
                                  unsigned const char *fieldName,
                                  unsigned const char *fieldValueFormat, ...) {
  char buffer[1024];

  va_list vaList;

  va_start(vaList, (const char *)fieldValueFormat);

  vsnprintf(buffer, sizeof(buffer), (const char *)fieldValueFormat, vaList);

  va_end(vaList);

  return addHeader(fieldName, (unsigned const char *)buffer, allocator);
}

}  // namespace ES
