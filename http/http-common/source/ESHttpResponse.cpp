#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

namespace ES {

HttpResponse::HttpResponse() : HttpMessage(), _statusCode(0), _reasonPhrase(0) {}

HttpResponse::~HttpResponse() {}

void HttpResponse::reset() {
  HttpMessage::reset();
  _statusCode = 0;
  _reasonPhrase = 0;
}

ESB::Error HttpResponse::copy(const HttpResponse *other, ESB::Allocator &allocator) {
  if (!other) {
    return ESB_NULL_POINTER;
  }

  setFlags(other->flags());
  setHttpVersion(other->httpVersion());
  setStatusCode(other->statusCode());
  setReasonPhrase(other->reasonPhrase());

  for (HttpHeader *header = (HttpHeader *)other->headers().first(); header; header = (HttpHeader *)header->next()) {
    ESB::Error error = addHeader(header, allocator);
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  return ESB_SUCCESS;
}

}  // namespace ES
