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

const HttpHeader *HttpMessage::findHeader(const char *fieldName) const {
  if (0 == fieldName) {
    return 0;
  }

  for (const HttpHeader *header = (const HttpHeader *)_headers.first(); header;
       header = (const HttpHeader *)header->next()) {
    if (0 == strcasecmp(fieldName, (const char *)header->fieldName())) {
      return header;
    }
  }

  return 0;
}

ESB::Error HttpMessage::addHeader(const char *fieldName, const char *fieldValue, ESB::Allocator &allocator) {
  if (!fieldName) {
    return ESB_NULL_POINTER;
  }

  int fieldNameLength = strlen((const char *)fieldName);
  int fieldValueLength = strlen((const char *)fieldValue);

  unsigned char *block = NULL;
  ESB::Error error = allocator.allocate(sizeof(HttpHeader) + fieldNameLength + fieldValueLength + 2, (void **)&block);
  if (ESB_SUCCESS != error) {
    return error;
  }

  memcpy(block + sizeof(HttpHeader), fieldName, fieldNameLength);
  block[sizeof(HttpHeader) + fieldNameLength] = 0;

  memcpy(block + sizeof(HttpHeader) + fieldNameLength + 1, fieldValue, fieldValueLength);
  block[sizeof(HttpHeader) + fieldNameLength + fieldValueLength + 1] = 0;

  HttpHeader *header =
      new (block) HttpHeader(block + sizeof(HttpHeader), block + sizeof(HttpHeader) + fieldNameLength + 1);
  _headers.addLast(header);

  return ESB_SUCCESS;
}

ESB::Error HttpMessage::addHeader(ESB::Allocator &allocator, const char *fieldName, const char *fieldValueFormat, ...) {
  char buffer[1024];
  va_list vaList;

  va_start(vaList, fieldValueFormat);
  vsnprintf(buffer, sizeof(buffer), fieldValueFormat, vaList);
  va_end(vaList);

  return addHeader(fieldName, buffer, allocator);
}

HttpMessage::HeaderCopyResult HttpMessage::HeaderCopyAll(const unsigned char *fieldName,
                                                         const unsigned char *fieldValue) {
  return HttpMessage::ES_HTTP_HEADER_COPY;
}

ESB::Error HttpMessage::copyHeaders(const ESB::EmbeddedList &headers, ESB::Allocator &allocator,
                                    HttpMessage::HeaderCopyFilter filter) {
  for (HttpHeader *header = (HttpHeader *)headers.first(); header; header = (HttpHeader *)header->next()) {
    ESB::Error error = ESB_SUCCESS;
    switch (filter(header->fieldName(), header->fieldValue())) {
      case ES_HTTP_HEADER_COPY:
        error = addHeader(header, allocator);
        if (ESB_SUCCESS != error) {
          return error;
        }
        break;
      case ES_HTTP_HEADER_SKIP:
        continue;
      case ES_HTTP_HEADER_COPY_AND_STOP:
        return addHeader(header, allocator);
      case ES_HTTP_HEADER_SKIP_AND_STOP:
        return ESB_SUCCESS;
      case ES_HTTP_HEADER_ERROR:
        return ESB_INVALID_FIELD;
    }
  }
  return ESB_SUCCESS;
}

}  // namespace ES
