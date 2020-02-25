#ifndef ES_HTTP_REQUEST_PARSER_H
#include <ESHttpRequestParser.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

#define ES_PARSING_METHOD (1 << 0)
#define ES_PARSING_REQUEST_URI (1 << 1)
#define ES_PARSING_HTTP_VERSION (1 << 2)
#define ES_PARSE_COMPLETE (1 << 3)

HttpRequestParser::HttpRequestParser(ESB::Buffer *workingBuffer,
                                     ESB::DiscardAllocator *allocator)
    : HttpMessageParser(workingBuffer, allocator),
      _requestState(0x00),
      _requestUriParser(workingBuffer, allocator) {}

HttpRequestParser::~HttpRequestParser() {}

void HttpRequestParser::reset() {
  HttpMessageParser::reset();

  _requestState = 0x00;
  _requestUriParser.reset();
}

ESB::Error HttpRequestParser::parseStartLine(ESB::Buffer *inputBuffer,
                                             HttpMessage *message) {
  // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

  if (ES_PARSE_COMPLETE & _requestState) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _requestState) {
    _requestState = ES_PARSING_METHOD;

    inputBuffer->readMark();
    _workingBuffer->clear();
  }

  HttpRequest *request = (HttpRequest *)message;

  ESB::Error error = ESB_SUCCESS;

  if (ES_PARSING_METHOD & _requestState) {
    error = parseMethod(inputBuffer, request);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _requestState &= ~ES_PARSING_METHOD;
    _requestState |= ES_PARSING_REQUEST_URI;

    inputBuffer->readMark();
    _workingBuffer->clear();
  }

  if (ES_PARSING_REQUEST_URI & _requestState) {
    error = _requestUriParser.parse(inputBuffer, request->getRequestUri());

    if (ESB_SUCCESS != error) {
      return error;
    }

    _requestState &= ~ES_PARSING_REQUEST_URI;
    _requestState |= ES_PARSING_HTTP_VERSION;

    inputBuffer->readMark();
    _workingBuffer->clear();
  }

  if (ES_PARSING_HTTP_VERSION & _requestState) {
    error = parseVersion(inputBuffer, request, false);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _requestState &= ~ES_PARSING_HTTP_VERSION;
    _requestState |= ES_PARSE_COMPLETE;

    inputBuffer->readMark();
    _workingBuffer->clear();

    return ESB_SUCCESS;
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpRequestParser::parseMethod(ESB::Buffer *inputBuffer,
                                          HttpRequest *request) {
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

  assert(ES_PARSING_METHOD & _requestState);

  unsigned char octet;

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (false == _workingBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->getNext();

    if (HttpUtil::IsSpace(octet)) {
      request->setMethod(_workingBuffer->duplicate(_allocator));

      return 0 == request->getMethod() ? ESB_OUT_OF_MEMORY : ESB_SUCCESS;
    }

    if (HttpUtil::IsToken(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    return ES_HTTP_BAD_REQUEST_METHOD;
  }
}

bool HttpRequestParser::isBodyNotAllowed(HttpMessage *message) {
  // A message-body MUST NOT be included in
  // a request if the specification of the request method (section 5.1.1)
  // does not allow sending an entity-body in requests. A server SHOULD
  // read and forward a message-body on any request; if the request method
  // does not include defined semantics for an entity-body, then the
  // message-body SHOULD be ignored when handling the request.

  HttpRequest *request = (HttpRequest *)message;

  if (0 == strcasecmp("GET", (const char *)request->getMethod()) ||
      0 == strcasecmp("DELETE", (const char *)request->getMethod())) {
    return true;
  }

  return false;
}

}  // namespace ES
