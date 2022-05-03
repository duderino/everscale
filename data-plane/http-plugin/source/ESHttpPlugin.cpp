#ifndef ES_HTTP_PLUGIN_H
#include <es_http_plugin.h>
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

using namespace ES;

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
    case HttpRequestUri::ES_URI_ASTERISK:
    case HttpRequestUri::ES_URI_HTTP:
    case HttpRequestUri::ES_URI_HTTPS:
    case HttpRequestUri::ES_URI_OTHER:
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
  return request ? (const es_http_request_uri_t)request->requestUri().type() : ES_HTTP_REQUEST_URI_OTHER;
}
