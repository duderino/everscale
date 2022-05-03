#ifndef ES_HTTP_PLUGIN_H
#include <es_http_plugin.h>
#endif

#include <gtest/gtest.h>

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

#ifndef ES_HTTP_HEADER_H
#include <ESHttpHeader.h>
#endif

#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

using namespace ES;

TEST(HttpPlugin, ReadHeader) {
  HttpHeader h("name", "value");

  es_http_header_t header = (es_http_header_t)&h;

  ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(header), "name"));
  ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(header), "value"));
}

TEST(HttpPlugin, RequestHeadersForwardIterate) {
  ESB::DiscardAllocator a(1024);
  HttpRequest r;

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_allocator_t allocator = (es_allocator_t)&a;

    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(request, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(request, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(request, allocator, "name3", "value3"));

    es_http_header_list_t headers = es_http_request_headers(request);

    es_http_header_t h1 = es_http_header_list_first(headers);
    ASSERT_TRUE(h1);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h1), "name1"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h1), "value1"));

    es_http_header_t h2 = es_http_header_next(h1);
    ASSERT_TRUE(h2);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h2), "name2"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h2), "value2"));

    es_http_header_t h3 = es_http_header_next(h2);
    ASSERT_TRUE(h3);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h3), "name3"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h3), "value3"));

    es_http_header_t h4 = es_http_header_next(h3);
    ASSERT_FALSE(h4);
  }
}

TEST(HttpPlugin, RequestHeadersReverseIterate) {
  ESB::DiscardAllocator a(1024);
  HttpRequest r;

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_allocator_t allocator = (es_allocator_t)&a;

    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(request, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(request, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(request, allocator, "name3", "value3"));

    es_http_header_list_t headers = es_http_request_headers(request);

    es_http_header_t h3 = es_http_header_list_last(headers);
    ASSERT_TRUE(h3);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h3), "name3"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h3), "value3"));

    es_http_header_t h2 = es_http_header_previous(h3);
    ASSERT_TRUE(h2);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h2), "name2"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h2), "value2"));

    es_http_header_t h1 = es_http_header_previous(h2);
    ASSERT_TRUE(h1);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h1), "name1"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h1), "value1"));

    es_http_header_t h0 = es_http_header_previous(h1);
    ASSERT_FALSE(h0);
  }
}

TEST(HttpPlugin, RequestUriType) {
  HttpRequest r;

  {
    es_http_request_t request = (es_http_request_t)&r;

    // HTTP is the default
    ASSERT_EQ(ES_HTTP_REQUEST_URI_HTTP, es_http_request_uri_type(request));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_REQUEST_URI_HTTPS));
    ASSERT_EQ(ES_HTTP_REQUEST_URI_HTTPS, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_REQUEST_URI_ASTERISK));
    ASSERT_EQ(ES_HTTP_REQUEST_URI_ASTERISK, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_REQUEST_URI_OTHER));
    ASSERT_EQ(ES_HTTP_REQUEST_URI_OTHER, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_REQUEST_URI_HTTP));
    ASSERT_EQ(ES_HTTP_REQUEST_URI_HTTP, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_INVALID_ARGUMENT, es_http_request_set_uri_type(request, (es_http_request_uri_t)42));
  }
}