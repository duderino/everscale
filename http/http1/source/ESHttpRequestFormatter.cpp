#ifndef ES_HTTP_REQUEST_FORMATTER_H
#include <ESHttpRequestFormatter.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

#define ES_FORMATTING_METHOD (1 << 0)
#define ES_FORMATTING_REQUEST_URI (1 << 1)
#define ES_FORMATTING_HTTP_VERSION (1 << 2)
#define ES_FORMAT_COMPLETE (1 << 3)

HttpRequestFormatter::HttpRequestFormatter() : _requestState(0x00), _requestUriFormatter() {}

HttpRequestFormatter::~HttpRequestFormatter() {}

void HttpRequestFormatter::reset() {
  HttpMessageFormatter::reset();

  _requestState = 0x00;
  _requestUriFormatter.reset();
}

ESB::Error HttpRequestFormatter::formatStartLine(ESB::Buffer *outputBuffer, const HttpMessage &message) {
  // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

  if (ES_FORMAT_COMPLETE & _requestState) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _requestState) {
    HttpUtil::Start(&_requestState, outputBuffer, ES_FORMATTING_METHOD);
  }

  HttpRequest &request = (HttpRequest &)message;

  ESB::Error error = ESB_SUCCESS;

  if (ES_FORMATTING_METHOD & _requestState) {
    error = formatMethod(outputBuffer, request);

    if (ESB_SUCCESS != error) {
      return error;
    }

    HttpUtil::Transition(&_requestState, outputBuffer, ES_FORMATTING_METHOD, ES_FORMATTING_REQUEST_URI);
  }

  if (ES_FORMATTING_REQUEST_URI & _requestState) {
    error = _requestUriFormatter.format(outputBuffer, request.requestUri());

    if (ESB_SUCCESS != error) {
      return error;
    }

    HttpUtil::Transition(&_requestState, outputBuffer, ES_FORMATTING_REQUEST_URI, ES_FORMATTING_HTTP_VERSION);
  }

  if (ES_FORMATTING_HTTP_VERSION & _requestState) {
    error = formatVersion(outputBuffer, request, true);

    if (ESB_SUCCESS != error) {
      return error;
    }

    return HttpUtil::Transition(&_requestState, outputBuffer, ES_FORMATTING_HTTP_VERSION, ES_FORMAT_COMPLETE);
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpRequestFormatter::formatMethod(ESB::Buffer *outputBuffer, const HttpRequest &request) {
  // Method                = "OPTIONS"                ; Section 9.2
  //                       | "GET"                    ; Section 9.3
  //                       | "HEAD"                   ; Section 9.4
  //                       | "POST"                   ; Section 9.5
  //                       | "PUT"                    ; Section 9.6
  //                       | "DELETE"                 ; Section 9.7
  //                       | "TRACE"                  ; Section 9.8
  //                       | "CONNECT"                ; Section 9.9
  //                       | extension-method
  // extension-method = token

  assert(ES_FORMATTING_METHOD & _requestState);

  if (0 == request.method() || 0 == request.method()[0]) {
    return ES_HTTP_BAD_REQUEST_METHOD;
  }

  for (const unsigned char *p = request.method(); *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    if (HttpUtil::IsToken(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_METHOD);
  }

  if (!outputBuffer->isWritable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext(' ');

  return ESB_SUCCESS;
}

}  // namespace ES
