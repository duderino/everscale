#ifndef ES_HTTP_PLUGIN_H
#include <ESHttpPlugin.h>
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

#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
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
  HttpRequest r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

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
  HttpRequest r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

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

TEST(HttpPlugin, RequestCopy) {
  HttpRequest s;
  HttpRequest d;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t src = (es_http_request_t)&s;
    es_http_request_t dest = (es_http_request_t)&d;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(src, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(src, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(src, allocator, "name3", "value3"));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_copy(src, dest, allocator, es_http_header_filter_copy_all, NULL));

    es_http_header_list_t headers = es_http_request_headers(dest);

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

static es_http_filter_result_t es_http_header_filter_copy_even(const char *name, const char *value, void *context) {
  int *count = (int *)context;
  (*count)++;

  return *count % 2 ? ES_HTTP_FILTER_SKIP : ES_HTTP_FILTER_COPY;
}

TEST(HttpPlugin, RequestFilteredCopy) {
  HttpRequest s;
  HttpRequest d;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t src = (es_http_request_t)&s;
    es_http_request_t dest = (es_http_request_t)&d;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;
    int count = 0;

    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(src, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(src, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_add_header(src, allocator, "name3", "value3"));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_copy(src, dest, allocator, es_http_header_filter_copy_even, &count));

    es_http_header_list_t headers = es_http_request_headers(dest);

    es_http_header_t h2 = es_http_header_list_last(headers);
    ASSERT_TRUE(h2);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h2), "name2"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h2), "value2"));

    es_http_header_t h1 = es_http_header_previous(h2);
    ASSERT_FALSE(h1);

    ASSERT_EQ(3, count);
  }
}

TEST(HttpPlugin, RequestUriType) {
  HttpRequest r;

  {
    es_http_request_t request = (es_http_request_t)&r;

    // HTTP is the default
    ASSERT_EQ(ES_HTTP_URI_HTTP, es_http_request_uri_type(request));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_URI_HTTPS));
    ASSERT_EQ(ES_HTTP_URI_HTTPS, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_URI_ASTERISK));
    ASSERT_EQ(ES_HTTP_URI_ASTERISK, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_URI_OTHER));
    ASSERT_EQ(ES_HTTP_URI_OTHER, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_URI_HTTP));
    ASSERT_EQ(ES_HTTP_URI_HTTP, es_http_request_uri_type(request));

    ASSERT_EQ(ESB_INVALID_ARGUMENT, es_http_request_set_uri_type(request, (es_http_request_uri_t)42));
  }
}

TEST(HttpPlugin, RequestUriPath) {
  HttpRequest r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_FALSE(es_http_request_uri_path(request));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_path(request, allocator, "/foo/bar/baz"));
    ASSERT_EQ(0, strcmp(es_http_request_uri_path(request), "/foo/bar/baz"));
  }
}

TEST(HttpPlugin, RequestUriQuery) {
  HttpRequest r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_FALSE(es_http_request_uri_query(request));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_query(request, allocator, "a=1&b=2&c=3"));
    ASSERT_EQ(0, strcmp(es_http_request_uri_query(request), "a=1&b=2&c=3"));
  }
}

TEST(HttpPlugin, RequestUriFragment) {
  HttpRequest r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_FALSE(es_http_request_uri_fragment(request));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_fragment(request, allocator, "anchor"));
    ASSERT_EQ(0, strcmp(es_http_request_uri_fragment(request), "anchor"));
  }
}

TEST(HttpPlugin, RequestUriHost) {
  HttpRequest r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_FALSE(es_http_request_uri_host(request));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_host(request, allocator, "example.com"));
    ASSERT_EQ(0, strcmp(es_http_request_uri_host(request), "example.com"));
  }
}

TEST(HttpPlugin, RequestUriPort) {
  HttpRequest r;

  {
    es_http_request_t request = (es_http_request_t)&r;

    ASSERT_EQ(-1, es_http_request_uri_port(request));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_port(request, 123));
    ASSERT_EQ(123, es_http_request_uri_port(request));

    ASSERT_EQ(ESB_INVALID_ARGUMENT, es_http_request_set_uri_port(request, -1));
    ASSERT_EQ(123, es_http_request_uri_port(request));

    ASSERT_EQ(ESB_INVALID_ARGUMENT, es_http_request_set_uri_port(request, 65536));
    ASSERT_EQ(123, es_http_request_uri_port(request));
  }
}

TEST(HttpPlugin, RequestUriOther) {
  HttpRequest r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);
  const char *other = "sip:1-999-123-4567@voip-provider.example.net";

  {
    es_http_request_t request = (es_http_request_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_FALSE(es_http_request_uri_other(request));
    ASSERT_EQ(ESB_INVALID_STATE, es_http_request_set_uri_other(request, allocator, other));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_type(request, ES_HTTP_URI_OTHER));
    ASSERT_EQ(ESB_SUCCESS, es_http_request_set_uri_other(request, allocator, other));
    ASSERT_EQ(0, strcmp(es_http_request_uri_other(request), other));
  }
}

TEST(HttpPlugin, ResponseHeadersForwardIterate) {
  HttpResponse r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_response_t response = (es_http_response_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(response, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(response, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(response, allocator, "name3", "value3"));

    es_http_header_list_t headers = es_http_response_headers(response);

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

TEST(HttpPlugin, ResponseHeadersReverseIterate) {
  HttpResponse r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_response_t response = (es_http_response_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(response, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(response, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(response, allocator, "name3", "value3"));

    es_http_header_list_t headers = es_http_response_headers(response);

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

TEST(HttpPlugin, ResponseCopy) {
  HttpResponse s;
  HttpResponse d;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_response_t src = (es_http_response_t)&s;
    es_http_response_t dest = (es_http_response_t)&d;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;

    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(src, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(src, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(src, allocator, "name3", "value3"));

    ASSERT_EQ(ESB_SUCCESS, es_http_response_copy(src, dest, allocator, es_http_header_filter_copy_all, NULL));

    es_http_header_list_t headers = es_http_response_headers(dest);

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

TEST(HttpPlugin, ResponseFilteredCopy) {
  HttpResponse s;
  HttpResponse d;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_response_t src = (es_http_response_t)&s;
    es_http_response_t dest = (es_http_response_t)&d;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;
    int count = 0;

    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(src, allocator, "name1", "value1"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(src, allocator, "name2", "value2"));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_add_header(src, allocator, "name3", "value3"));

    ASSERT_EQ(ESB_SUCCESS, es_http_response_copy(src, dest, allocator, es_http_header_filter_copy_even, &count));

    es_http_header_list_t headers = es_http_response_headers(dest);

    es_http_header_t h2 = es_http_header_list_last(headers);
    ASSERT_TRUE(h2);
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_name(h2), "name2"));
    ASSERT_TRUE(0 == ::strcasecmp(es_http_header_value(h2), "value2"));

    es_http_header_t h1 = es_http_header_previous(h2);
    ASSERT_FALSE(h1);

    ASSERT_EQ(3, count);
  }
}

TEST(HttpPlugin, ResponseStatusCode) {
  HttpResponse r;

  {
    es_http_response_t response = (es_http_response_t)&r;

    ASSERT_EQ(-1, es_http_response_status_code(response));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_set_status_code(response, 999));
    ASSERT_EQ(999, es_http_response_status_code(response));

    ASSERT_EQ(ESB_INVALID_ARGUMENT, es_http_response_set_status_code(response, -1));
    ASSERT_EQ(999, es_http_response_status_code(response));

    ASSERT_EQ(ESB_INVALID_ARGUMENT, es_http_response_set_status_code(response, 1000));
    ASSERT_EQ(999, es_http_response_status_code(response));
  }
}

TEST(HttpPlugin, ResponseReasonPhrase) {
  HttpResponse r;
  ESB::DiscardAllocator a(1024, sizeof(ESB::Word), 1, ESB::SystemAllocator::Instance(), true);

  {
    es_http_response_t response = (es_http_response_t)&r;
    es_http_allocator_t allocator = (es_http_allocator_t)&a;
    const char *reason_phrase = es_http_response_default_reason_phrase(404, NULL);

    ASSERT_EQ(0, strcmp(reason_phrase, "Not Found"));

    ASSERT_FALSE(es_http_response_reason_phrase(response));
    ASSERT_EQ(ESB_SUCCESS, es_http_response_set_reason_phrase(response, allocator, reason_phrase));
    ASSERT_EQ(0, strcmp(es_http_response_reason_phrase(response), reason_phrase));
  }
}