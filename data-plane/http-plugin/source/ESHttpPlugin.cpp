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

void es_describe_error(es_http_error_t error, char *buffer, int size) { ESB::DescribeError(error, buffer, size); }

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

es_http_header_filter_result_t es_http_header_filter_copy_all(const char *name, const char *value, void *context) {
  return ES_HEADER_COPY;
}

es_http_error_t es_http_request_copy(const es_http_request_t s, es_http_request_t d, es_allocator_t a,
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

es_http_error_t es_http_request_add_header(es_http_request_t r, es_allocator_t a, const char *name, const char *value) {
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
    case HttpRequestUri::UriType::ES_URI_ASTERISK:
    case HttpRequestUri::UriType::ES_URI_HTTP:
    case HttpRequestUri::UriType::ES_URI_HTTPS:
    case HttpRequestUri::UriType::ES_URI_OTHER:
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
  return request ? (const es_http_request_uri_t)request->requestUri().type() : ES_URI_OTHER;
}

es_http_error_t es_http_request_set_uri_path(es_http_request_t r, es_allocator_t a, const char *path) {
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

es_http_error_t es_http_request_set_uri_query(es_http_request_t r, es_allocator_t a, const char *query) {
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

es_http_error_t es_http_request_set_uri_fragment(es_http_request_t r, es_allocator_t a, const char *fragment) {
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

es_http_error_t es_http_request_set_uri_host(es_http_request_t r, es_allocator_t a, const char *host) {
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

es_http_error_t es_http_request_set_uri_other(es_http_request_t r, es_allocator_t a, const char *other) {
  if (!r || !a || !other) {
    return ESB_NULL_POINTER;
  }

  HttpRequest *request = (HttpRequest *)r;

  if (HttpRequestUri::UriType::ES_URI_OTHER != request->requestUri().type()) {
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
