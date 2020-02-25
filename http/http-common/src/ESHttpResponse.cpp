#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

namespace ES {

HttpResponse::HttpResponse()
    : HttpMessage(), _statusCode(0), _reasonPhrase(0) {}

HttpResponse::~HttpResponse() {}

void HttpResponse::reset() {
  HttpMessage::reset();
  _statusCode = 0;
  _reasonPhrase = 0;
}

}  // namespace ES
