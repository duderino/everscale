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

// https://datatracker.ietf.org/doc/html/rfc7231#section-6.2

static const char *OneHundredClass[]{
    "Continue",            // 100
    "Switching Protocols"  // 101
};

static const char *TwoHundredClass[]{"OK",  // 200
                                     "Created",    "Accepted",     "Non-Authoritative Information",
                                     "No Content", "Reset Content"};

static const char *ThreeHundredClass[]{"Multiple Choices",  // 300
                                       "Moved Permanently",
                                       "Found",
                                       "See Other",
                                       NULL,  // 304
                                       "Use Proxy",
                                       "Switch Proxy",
                                       "Temporary Redirect"};

static const char *FourHundredClass[] = {"Bad Request",  // 400
                                         NULL,
                                         "Payment Required",
                                         "Forbidden",
                                         "Not Found",
                                         "Method Not Allowed",
                                         "Not Acceptable",
                                         NULL,  // 407
                                         "Request Timeout",
                                         "Conflict",
                                         "Gone",
                                         "Length Required",
                                         NULL,  // 412
                                         "Payload Too Large",
                                         "URI Too Long",
                                         "Unsupported Media Type",
                                         NULL,  // 416
                                         "Expectation Failed",
                                         NULL,  // 418
                                         NULL,  // 419
                                         NULL,  // 420,
                                         NULL,  // 421
                                         NULL,  // 422
                                         NULL,  // 423
                                         NULL,  // 424
                                         NULL,  // 425
                                         "Upgrade Required"};

static const char *FiveHundredClass[] = {
    "Internal Server Error",  // 500
    "Not Implemented",       "Bad Gateway", "Service Unavailable", "Gateway Timeout", "HTTP Version Not Supported"};

const char *HttpResponse::DefaultReasonPhrase(int statusCode) {
  if (100 > statusCode || 600 <= statusCode) {
    return NULL;
  }

  if (200 > statusCode) {
    return (((int)sizeof(OneHundredClass) / sizeof(char *)) > statusCode - 100) ? OneHundredClass[statusCode - 100]
                                                                                : NULL;
  }

  if (300 > statusCode) {
    return (((int)sizeof(TwoHundredClass) / sizeof(char *)) > statusCode - 200) ? TwoHundredClass[statusCode - 200]
                                                                                : NULL;
  }

  if (400 > statusCode) {
    return (((int)sizeof(ThreeHundredClass) / sizeof(char *)) > statusCode - 300) ? ThreeHundredClass[statusCode - 300]
                                                                                  : NULL;
  }

  if (500 > statusCode) {
    return (((int)sizeof(FourHundredClass) / sizeof(char *)) > statusCode - 400) ? FourHundredClass[statusCode - 400]
                                                                                 : NULL;
  }

  return (((int)sizeof(FiveHundredClass) / sizeof(char *)) > statusCode - 500) ? FiveHundredClass[statusCode - 500]
                                                                               : NULL;
}

}  // namespace ES
