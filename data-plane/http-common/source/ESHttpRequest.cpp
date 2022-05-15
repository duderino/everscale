#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

namespace ES {

HttpRequest::HttpRequest() : HttpMessage(), _method(0), _requestUri() {}

HttpRequest::~HttpRequest() {}

void HttpRequest::reset() {
  HttpMessage::reset();
  _method = 0;
  _requestUri.reset();
}

ESB::Error HttpRequest::copy(const ES::HttpRequest *other, ESB::Allocator &allocator, es_http_header_filter filter,
                             void *context) {
  if (!other) {
    return ESB_NULL_POINTER;
  }

  setFlags(other->flags());
  setHttpVersion(other->httpVersion());
  setMethod(other->method());

  ESB::Error error = _requestUri.copy(&other->requestUri(), allocator);

  if (ESB_SUCCESS != error) {
    return error;
  }

  return copyHeaders(other->headers(), allocator, filter, context);
}

ESB::Error HttpRequest::parsePeerAddress(char *hostname, int size, ESB::UInt16 *port, bool *isSecure) const {
  if (!hostname || !port || !isSecure) {
    return ESB_NULL_POINTER;
  }

  if (0 >= size) {
    return ESB_INVALID_ARGUMENT;
  }

  /* 5.2 The Resource Identified by a Request
   *
   *   The exact resource identified by an Internet request is determined by
   *   examining both the Request-URI and the Host header field.
   *
   *   An origin server that does not allow resources to differ by the
   *   requested host MAY ignore the Host header field value when
   *   determining the resource identified by an HTTP/1.1 request. (But see
   *   section 19.6.1.1 for other requirements on Host support in HTTP/1.1.)
   *
   *   An origin server that does differentiate resources based on the host
   *   requested (sometimes referred to as virtual hosts or vanity host
   *   names) MUST use the following rules for determining the requested
   *   resource on an HTTP/1.1 request:
   *
   *   1. If Request-URI is an absoluteURI, the host is part of the
   *     Request-URI. Any Host header field value in the request MUST be
   *     ignored.
   *
   *   2. If the Request-URI is not an absoluteURI, and the request includes
   *     a Host header field, the host is determined by the Host header
   *     field value.
   *
   *   3. If the host as determined by rule 1 or 2 is not a valid host on
   *     the server, the response MUST be a 400 (Bad Request) error message.
   *
   *   Recipients of an HTTP/1.0 request that lacks a Host header field MAY
   *   attempt to use heuristics (e.g., examination of the URI path for
   *   something unique to a particular host) in order to determine what
   *   exact resource is being requested.
   */

  *isSecure = HttpRequestUri::UriType::ES_HTTP_URI_HTTPS == _requestUri.type();

  if (_requestUri.host()) {
    int n = strlen((const char *)_requestUri.host());

    if (n >= size) {
      return ESB_OVERFLOW;
    }

    memcpy(hostname, _requestUri.host(), n);
    hostname[n] = 0;

    if (0 >= _requestUri.port()) {
      *port = *isSecure ? 443 : 80;
    } else {
      *port = _requestUri.port();
    }

    return ESB_SUCCESS;
  }

  const HttpHeader *header = findHeader("Host");

  if (!header || !header->fieldValue()) {
    return ESB_INVALID_ARGUMENT;
  }

  const unsigned char *p = header->fieldValue();
  const unsigned char *q = p;

  while (*p && ':' != *p) {
    ++p;
  }

  int n = p - q;

  if (n >= size) {
    return ESB_OVERFLOW;
  }

  memcpy(hostname, q, n);
  hostname[n] = 0;

  if (':' != *p) {
    *port = *isSecure ? 443 : 80;
  } else {
    ++p;

    ESB::UInt16 value = 0;

    while (*p && HttpUtil::IsDigit(*p)) {
      value = value * 10 + (*p - '0');
      ++p;
    }

    if (0 == value) {
      *port = *isSecure ? 443 : 80;
    } else {
      *port = value;
    }
  }

  return ESB_SUCCESS;
}

}  // namespace ES
