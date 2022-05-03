#ifndef ES_HTTP_PLUGIN_H
#define ES_HTTP_PLUGIN_H

extern "C" {

typedef int es_http_error_t;
typedef struct es_allocator *es_allocator_t;

// Header

typedef struct es_http_header *es_http_header_t;
const char *es_http_header_name(const es_http_header_t header);
const char *es_http_header_value(const es_http_header_t header);
const es_http_header_t es_http_header_next(const es_http_header_t header);
const es_http_header_t es_http_header_previous(const es_http_header_t header);

// Header List

typedef struct es_http_header_list *es_http_header_list_t;
const es_http_header_t es_http_header_list_first(const es_http_header_list_t headers);
const es_http_header_t es_http_header_list_last(const es_http_header_list_t headers);

// Request

typedef enum {
  ES_HTTP_REQUEST_URI_ASTERISK = 0, /**< OPTIONS * HTTP/1.1 */
  ES_HTTP_REQUEST_URI_HTTP = 1,     /**< GET http://www.yahoo.com/ HTTP/1.1 */
  ES_HTTP_REQUEST_URI_HTTPS = 2,    /**< POST https://www.yahoo.com/ HTTP/1.1 */
  ES_HTTP_REQUEST_URI_OTHER = 3     /**< POST foo://opaque */
} es_http_request_uri_t;

typedef struct es_http_request *es_http_request_t;
const es_http_header_list_t es_http_request_headers(const es_http_request_t request);
es_http_error_t es_http_request_add_header(es_http_request_t request, es_allocator_t allocator, const char *name,
                                           const char *value);
es_http_error_t es_http_request_set_uri_type(es_http_request_t request, es_http_request_uri_t type);
es_http_request_uri_t es_http_request_uri_type(const es_http_request_t request);
}

#endif