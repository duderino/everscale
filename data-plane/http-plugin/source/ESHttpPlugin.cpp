#ifndef ES_HTTP_PLUGIN_H
#include <ESHttpPlugin.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

using namespace ES;

es_http_error_t ES_HTTP_SUCCESS = ESB_SUCCESS;
es_http_error_t ES_HTTP_OTHER_ERROR = ESB_OTHER_ERROR;
es_http_error_t ES_HTTP_OPERATION_NOT_SUPPORTED = ESB_OPERATION_NOT_SUPPORTED;
es_http_error_t ES_HTTP_NULL_POINTER = ESB_NULL_POINTER;
es_http_error_t ES_HTTP_UNIQUENESS_VIOLATION = ESB_UNIQUENESS_VIOLATION;
es_http_error_t ES_HTTP_INVALID_ARGUMENT = ESB_INVALID_ARGUMENT;
es_http_error_t ES_HTTP_OUT_OF_MEMORY = ESB_OUT_OF_MEMORY;
es_http_error_t ES_HTTP_NOT_INITIALIZED = ESB_NOT_INITIALIZED;
es_http_error_t ES_HTTP_AGAIN = ESB_AGAIN;
es_http_error_t ES_HTTP_INTR = ESB_INTR;
es_http_error_t ES_HTTP_INPROGRESS = ESB_INPROGRESS;
es_http_error_t ES_HTTP_TIMEOUT = ESB_TIMEOUT;
es_http_error_t ES_HTTP_ARGUMENT_TOO_SHORT = ESB_ARGUMENT_TOO_SHORT;
es_http_error_t ES_HTTP_RESULT_TRUNCATED = ESB_RESULT_TRUNCATED;
es_http_error_t ES_HTTP_INVALID_STATE = ESB_INVALID_STATE;
es_http_error_t ES_HTTP_NOT_OWNER = ESB_NOT_OWNER;
es_http_error_t ES_HTTP_IN_USE = ESB_IN_USE;
es_http_error_t ES_HTTP_CANNOT_FIND = ESB_CANNOT_FIND;
es_http_error_t ES_HTTP_INVALID_ITERATOR = ESB_INVALID_ITERATOR;
es_http_error_t ES_HTTP_OVERFLOW = ESB_OVERFLOW;
es_http_error_t ES_HTTP_CANNOT_CONVERT = ESB_CANNOT_CONVERT;
es_http_error_t ES_HTTP_ILLEGAL_ENCODING = ESB_ILLEGAL_ENCODING;
es_http_error_t ES_HTTP_UNSUPPORTED_CHARSET = ESB_UNSUPPORTED_CHARSET;
es_http_error_t ES_HTTP_OUT_OF_BOUNDS = ESB_OUT_OF_BOUNDS;
es_http_error_t ES_HTTP_SHUTDOWN = ESB_SHUTDOWN;
es_http_error_t ES_HTTP_CLEANUP = ESB_CLEANUP;
es_http_error_t ES_HTTP_PAUSE = ESB_PAUSE;
es_http_error_t ES_HTTP_SEND_RESPONSE = ESB_SEND_RESPONSE;
es_http_error_t ES_HTTP_CLOSED = ESB_CLOSED;
es_http_error_t ES_HTTP_NOT_IMPLEMENTED = ESB_NOT_IMPLEMENTED;
es_http_error_t ES_HTTP_BREAK = ESB_BREAK;
es_http_error_t ES_HTTP_UNSUPPORTED_TRANSPORT = ESB_UNSUPPORTED_TRANSPORT;
es_http_error_t ES_HTTP_TLS_HANDSHAKE_ERROR = ESB_TLS_HANDSHAKE_ERROR;
es_http_error_t ES_HTTP_TLS_SESSION_ERROR = ESB_TLS_SESSION_ERROR;
es_http_error_t ES_HTTP_GENERAL_TLS_ERROR = ESB_GENERAL_TLS_ERROR;
es_http_error_t ES_HTTP_UNDERFLOW = ESB_UNDERFLOW;
es_http_error_t ES_HTTP_CANNOT_PARSE = ESB_CANNOT_PARSE;
es_http_error_t ES_HTTP_MISSING_FIELD = ESB_MISSING_FIELD;
es_http_error_t ES_HTTP_INVALID_FIELD = ESB_INVALID_FIELD;

void es_http_describe_error(es_http_error_t error, char *buffer, int size) { ESB::DescribeError(error, buffer, size); }

const char *es_http_header_name(const es_http_header_t h) {
  const HttpHeader *header = (const HttpHeader *)h;
  return header ? (const char *)header->fieldName() : NULL;
}

const char *es_http_header_value(const es_http_header_t h) {
  const HttpHeader *header = (const HttpHeader *)h;
  return header ? (const char *)header->fieldValue() : NULL;
}

const es_http_header_t es_http_header_next(const es_http_header_t h) {
  const HttpHeader *header = (const HttpHeader *)h;
  return header ? (const es_http_header_t)header->next() : NULL;
}

const es_http_header_t es_http_header_previous(const es_http_header_t h) {
  const HttpHeader *header = (const HttpHeader *)h;
  return header ? (const es_http_header_t)header->previous() : NULL;
}

const es_http_header_t es_http_header_list_first(const es_http_header_list_t h) {
  const ESB::EmbeddedList *list = (const ESB::EmbeddedList *)h;
  return list ? (const es_http_header_t)list->first() : NULL;
}

const es_http_header_t es_http_header_list_last(const es_http_header_list_t h) {
  const ESB::EmbeddedList *list = (const ESB::EmbeddedList *)h;
  return list ? (const es_http_header_t)list->last() : NULL;
}

es_http_filter_result_t es_http_header_filter_copy_all(const char *name, const char *value, void *context) {
  return ES_HTTP_FILTER_COPY;
}

es_http_error_t es_http_request_copy(const es_http_request_t s, es_http_request_t d, es_http_allocator_t a,
                                     es_http_header_filter filter, void *context) {
  if (!s || !d || !a) {
    return ESB_NULL_POINTER;
  }

  if (!filter) {
    filter = es_http_header_filter_copy_all;
  }

  const HttpRequest *source = (const HttpRequest *)s;
  HttpRequest *dest = (HttpRequest *)d;
  ESB::Allocator *allocator = (ESB::Allocator *)a;

  return dest->copy(source, *allocator, filter, context);
}

const es_http_header_list_t es_http_request_headers(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (const es_http_header_list_t)&request->headers() : NULL;
}

es_http_error_t es_http_request_add_header(es_http_request_t r, es_http_allocator_t a, const char *name,
                                           const char *value) {
  if (!r || !a || !name || !value) {
    return ESB_NULL_POINTER;
  }

  HttpRequest *request = (HttpRequest *)r;
  ESB::Allocator *allocator = (ESB::Allocator *)a;
  return (es_http_error_t)request->addHeader(name, value, *allocator);
}

es_http_error_t es_http_request_set_uri_type(es_http_request_t r, es_http_request_uri_t t) {
  if (!r) {
    return ESB_NULL_POINTER;
  }

  HttpRequestUri::UriType type = (HttpRequestUri::UriType)t;

  switch (type) {
    case HttpRequestUri::UriType::ES_HTTP_URI_ASTERISK:
    case HttpRequestUri::UriType::ES_HTTP_URI_HTTP:
    case HttpRequestUri::UriType::ES_HTTP_URI_HTTPS:
    case HttpRequestUri::UriType::ES_HTTP_URI_OTHER:
      break;
    default:
      return ESB_INVALID_ARGUMENT;
  }

  HttpRequest *request = (HttpRequest *)r;
  request->requestUri().setType(type);
  return ESB_SUCCESS;
}

es_http_request_uri_t es_http_request_uri_type(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (const es_http_request_uri_t)request->requestUri().type() : ES_HTTP_URI_OTHER;
}

es_http_error_t es_http_request_set_uri_path(es_http_request_t r, es_http_allocator_t a, const char *path) {
  if (!r || !a || !path) {
    return ESB_NULL_POINTER;
  }

  HttpRequest *request = (HttpRequest *)r;
  ESB::Allocator *allocator = (ESB::Allocator *)a;

  char *copy = NULL;
  ESB::Error error = ESB::Duplicate(path, *allocator, &copy);
  if (ESB_SUCCESS != error) {
    return error;
  }

  request->requestUri().setAbsPath(copy);
  return ESB_SUCCESS;
}

const char *es_http_request_uri_path(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (const char *)request->requestUri().absPath() : NULL;
}

es_http_error_t es_http_request_set_uri_query(es_http_request_t r, es_http_allocator_t a, const char *query) {
  if (!r || !a || !query) {
    return ESB_NULL_POINTER;
  }

  HttpRequest *request = (HttpRequest *)r;
  ESB::Allocator *allocator = (ESB::Allocator *)a;

  char *copy = NULL;
  ESB::Error error = ESB::Duplicate(query, *allocator, &copy);
  if (ESB_SUCCESS != error) {
    return error;
  }

  request->requestUri().setQuery(copy);
  return ESB_SUCCESS;
}

const char *es_http_request_uri_query(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (const char *)request->requestUri().query() : NULL;
}

es_http_error_t es_http_request_set_uri_fragment(es_http_request_t r, es_http_allocator_t a, const char *fragment) {
  if (!r || !a || !fragment) {
    return ESB_NULL_POINTER;
  }

  HttpRequest *request = (HttpRequest *)r;
  ESB::Allocator *allocator = (ESB::Allocator *)a;

  char *copy = NULL;
  ESB::Error error = ESB::Duplicate(fragment, *allocator, &copy);
  if (ESB_SUCCESS != error) {
    return error;
  }

  request->requestUri().setFragment(copy);
  return ESB_SUCCESS;
}

const char *es_http_request_uri_fragment(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (const char *)request->requestUri().fragment() : NULL;
}

es_http_error_t es_http_request_set_uri_host(es_http_request_t r, es_http_allocator_t a, const char *host) {
  if (!r || !a || !host) {
    return ESB_NULL_POINTER;
  }

  HttpRequest *request = (HttpRequest *)r;
  ESB::Allocator *allocator = (ESB::Allocator *)a;

  char *copy = NULL;
  ESB::Error error = ESB::Duplicate(host, *allocator, &copy);
  if (ESB_SUCCESS != error) {
    return error;
  }

  request->requestUri().setHost(copy);
  return ESB_SUCCESS;
}

const char *es_http_request_uri_host(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (const char *)request->requestUri().host() : NULL;
}

es_http_error_t es_http_request_set_uri_port(es_http_request_t r, int32_t port) {
  HttpRequest *request = (HttpRequest *)r;
  if (0 > port || 65535 < port) {
    return ESB_INVALID_ARGUMENT;
  }
  request->requestUri().setPort(port);
  return ESB_SUCCESS;
}

int32_t es_http_request_uri_port(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (int16_t)request->requestUri().port() : -1;
}

es_http_error_t es_http_request_set_uri_other(es_http_request_t r, es_http_allocator_t a, const char *other) {
  if (!r || !a || !other) {
    return ESB_NULL_POINTER;
  }

  HttpRequest *request = (HttpRequest *)r;

  if (HttpRequestUri::UriType::ES_HTTP_URI_OTHER != request->requestUri().type()) {
    return ESB_INVALID_STATE;
  }

  ESB::Allocator *allocator = (ESB::Allocator *)a;

  char *copy = NULL;
  ESB::Error error = ESB::Duplicate(other, *allocator, &copy);
  if (ESB_SUCCESS != error) {
    return error;
  }

  request->requestUri().setOther(copy);
  return ESB_SUCCESS;
}

const char *es_http_request_uri_other(const es_http_request_t r) {
  const HttpRequest *request = (const HttpRequest *)r;
  return request ? (const char *)request->requestUri().other() : NULL;
}
