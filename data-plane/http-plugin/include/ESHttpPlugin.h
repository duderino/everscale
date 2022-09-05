#ifndef ES_HTTP_PLUGIN_H
#define ES_HTTP_PLUGIN_H

#include <stdint.h>

extern "C" {

//
// Error
//

typedef int es_http_error_t;

/**
 * Operation was successful.
 */
extern es_http_error_t ES_HTTP_SUCCESS;

/**
 * An uncategorized error occurred
 */
extern es_http_error_t ES_HTTP_OTHER_ERROR;

/**
 * Function does not implement this operation
 */
extern es_http_error_t ES_HTTP_OPERATION_NOT_SUPPORTED;

/**
 * Operation encountered a NULL pointer where it shouldn't have
 */
extern es_http_error_t ES_HTTP_NULL_POINTER;

/**
 * Operation would have produced multiple keys with the same value
 */
extern es_http_error_t ES_HTTP_UNIQUENESS_VIOLATION;

/**
 * Operation passed an argument with an invalid value
 */
extern es_http_error_t ES_HTTP_INVALID_ARGUMENT;

/**
 * Operation failed to allocate necessary memory
 */
extern es_http_error_t ES_HTTP_OUT_OF_MEMORY;

/**
 * Operation performed on an object that has not been initialized or whose intialization failed.
 */
extern es_http_error_t ES_HTTP_NOT_INITIALIZED;

/**
 * Operation could not be completed now, but should be attempted again later.
 */
extern es_http_error_t ES_HTTP_AGAIN;

/**
 * Operation was interrupted before it could complete
 */
extern es_http_error_t ES_HTTP_INTR;

/**
 * Operation is still in progress
 */
extern es_http_error_t ES_HTTP_INPROGRESS;

/**
 * Operation timed out before it could complete
 */
extern es_http_error_t ES_HTTP_TIMEOUT;

/**
 * Operation was passed an argument that was not long enough
 *
 */
extern es_http_error_t ES_HTTP_ARGUMENT_TOO_SHORT;

/**
 * Operation was successful, but result was partially or fully truncated
 */
extern es_http_error_t ES_HTTP_RESULT_TRUNCATED;

/**
 * Object is in a state where it cannot perform the operation
 */
extern es_http_error_t ES_HTTP_INVALID_STATE;

/**
 * Object does not own the requested resource.
 */
extern es_http_error_t ES_HTTP_NOT_OWNER;

/**
 * Object has resources currently in use.
 */
extern es_http_error_t ES_HTTP_IN_USE;

/**
 * Operation cannot find the specified element
 */
extern es_http_error_t ES_HTTP_CANNOT_FIND;

/**
 * Operation passed an invalid iterator
 */
extern es_http_error_t ES_HTTP_INVALID_ITERATOR;

/**
 * Operation cannot be completed without exceeding a hard limit.
 */
extern es_http_error_t ES_HTTP_OVERFLOW;

/**
 * Operation cannot perform requested conversion.
 */
extern es_http_error_t ES_HTTP_CANNOT_CONVERT;

/**
 * Operation was passed an argument with an illegal encoding.
 */
extern es_http_error_t ES_HTTP_ILLEGAL_ENCODING;

/**
 * Operation was passed an unsupported character set or encoding.
 */
extern es_http_error_t ES_HTTP_UNSUPPORTED_CHARSET;

/**
 * Operation was passed an index outside of the bounds of an array.
 */
extern es_http_error_t ES_HTTP_OUT_OF_BOUNDS;

/**
 * Operation cannot be completed because shutdown has already been called.
 */
extern es_http_error_t ES_HTTP_SHUTDOWN;

/**
 * Operation is finished and should be cleaned up
 */
extern es_http_error_t ES_HTTP_CLEANUP;

/**
 * Operation has been paused
 */
extern es_http_error_t ES_HTTP_PAUSE;

/**
 * Send a response immediately
 */
extern es_http_error_t ES_HTTP_SEND_RESPONSE;

/**
 * Close the socket immediately
 */
extern es_http_error_t ES_HTTP_CLOSED;

/**
 * Operation has not been implemented
 */
extern es_http_error_t ES_HTTP_NOT_IMPLEMENTED;

/**
 * Do not call operation again
 */
extern es_http_error_t ES_HTTP_BREAK;

/**
 * Operation was passed an unsupported transport
 */
extern es_http_error_t ES_HTTP_UNSUPPORTED_TRANSPORT;

/**
 * The TLS handshake did not complete successfully (invalid certs, etc)
 */
extern es_http_error_t ES_HTTP_TLS_HANDSHAKE_ERROR;

/**
 * A non-recoverable error occurred during the post-handshake TLS session
 */
extern es_http_error_t ES_HTTP_TLS_SESSION_ERROR;

/**
 * A general TLS error not tied to a specific session has occurred
 */
extern es_http_error_t ES_HTTP_GENERAL_TLS_ERROR;

/**
 * Operation cannot be completed without falling below a valid threshold.
 */
extern es_http_error_t ES_HTTP_UNDERFLOW;

/**
 * Document cannot be successfully parsed.
 */
extern es_http_error_t ES_HTTP_CANNOT_PARSE;

/**
 * Document is missing a mandatory field.
 */
extern es_http_error_t ES_HTTP_MISSING_FIELD;

/**
 * Document has a field with an invalid value.
 */
extern es_http_error_t ES_HTTP_INVALID_FIELD;

/**
 * Convert an es_http_error_t to a string description
 *
 * @param error An es_http_error_t error code
 * @param buffer A buffer to store the string description
 * @param size The size of the buffer - this function will always NULL terminate the buffer as long as the buffer size
 * is at least 1.
 */
void es_http_describe_error(es_http_error_t error, char *buffer, int size);

//
// Allocator
//

typedef struct es_http_allocator *es_http_allocator_t;

//
// L3 IP Address
//

/**
 * An L3 IP address
 */
typedef struct es_http_address *es_http_address_t;

/**
 * Set the ip and port with a sockaddr struct
 *
 * @param address The es_http_address_t to modify
 * @param sockaddr The sockaddr to copy
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise (won't fail unless you pass in a NULL)
 */
extern es_http_error_t es_http_address_set_sockaddr(es_http_address_t address, const struct sockaddr_in *sockaddr);

/**
 * Set the ip address
 *
 * @param address The es_http_address_t to modify
 * @param presentation The IP address string in presentation/dotted IP format
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise (will only fail if you pass in NULL or an invalid
 * presentation address)
 */
extern es_http_error_t es_http_address_set_ip(es_http_address_t address, const char *presentation);

/**
 * Set the port
 *
 * @param address The es_http_address_t to modify
 * @param port The port
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise (won't fail unless you pass in a NULL)
 */
extern es_http_error_t es_http_address_set_port(es_http_address_t address, uint16_t port);

/**
 * Convert the address to a sockaddr struct
 *
 * @param address The es_http_address_t to read
 * @param sockaddr The sockaddr struct
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise (won't fail unless you pass in a NULL)
 */
extern es_http_error_t es_http_address_sockaddr(const es_http_address_t address, struct sockaddr_in *sockaddr);

/**
 * Read the ip address from a es_http_address_t
 *
 * @param address The es_http_address_t to read
 * @param presentation The buffer to hold the presentation address
 * @param size The size of the presentation buffer.  Should be at least INET6_ADDRSTRLEN (46)
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise (won't fail unless you pass in a NULL or a buffer
 * that's < INET6_ADDRSTRLEN)
 */
extern es_http_error_t es_http_address_ip(const es_http_address_t address, char *presentation, int size);

/**
 * Read the port from a es_http_address_t
 *
 * @param address The es_http_address_t to read
 * @param port The port to read
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise (won't fail unless you pass in a NULL)
 */
extern es_http_error_t es_http_address_port(const es_http_address_t address, uint16_t *port);

//
// Header
//

/**
 * A single HTTP header
 */
typedef struct es_http_header *es_http_header_t;

/**
 * Get the field name of a HTTP header.
 *
 * @param header The HTTP header
 * @return The field name of the HTTP header
 */
const char *es_http_header_name(const es_http_header_t header);

/**
 * Get the field value of a HTTP header.
 *
 * @param header The HTTP header
 * @return The field value of the HTTP header
 */
const char *es_http_header_value(const es_http_header_t header);

/**
 * Get the next HTTP header in a linked list of HTTP headers.
 *
 * @param header The HTTP header
 * @return The next HTTP header in the list or NULL if at the end of the list.
 */
const es_http_header_t es_http_header_next(const es_http_header_t header);

/**
 * Get the previous HTTP header in a linked list of HTTP headers.
 *
 * @param header The HTTP header
 * @return The previous HTTP header in the list or NULL if at the beginning of the list.
 */
const es_http_header_t es_http_header_previous(const es_http_header_t header);

//
// Header List
//

/**
 * A linked list of HTTP headers
 */
typedef struct es_http_header_list *es_http_header_list_t;

/**
 * Get the first HTTP header in a linked list of HTTP headers
 *
 * @param headers The HTTP header list
 * @return THe first HTTP header in the list or NULL if the list is empty.
 */
const es_http_header_t es_http_header_list_first(const es_http_header_list_t headers);

/**
 * Get the last HTTP header in a linkd list of HTTP headers
 *
 * @param headers The HTTP header list
 * @return The last HTTP header in the list or NULL if the list is empty.
 */
const es_http_header_t es_http_header_list_last(const es_http_header_list_t headers);

/**
 * The return value of a copy filter
 */
typedef enum {
  ES_HTTP_FILTER_COPY = 0,          /**< Copy the header */
  ES_HTTP_FILTER_SKIP = 1,          /**< Skip the header */
  ES_HTTP_FILTER_COPY_AND_STOP = 2, /**< Copy the header and stop processing more headers */
  ES_HTTP_FILTER_SKIP_AND_STOP = 3, /**< Skip the header and stop processing more headers */
  ES_HTTP_FILTER_ERROR = 4          /**< Send a 400 bad response */
} es_http_filter_result_t;

/**
 * A header copy filter
 *
 * @param name The field name of the HTTP header
 * @param value The field value of the HTTP header
 * @param context The caller-supplied context if any
 * @return A es_http_filter_result_t that controls the iteration
 */
typedef es_http_filter_result_t (*es_http_header_filter)(const char *name, const char *value, void *context);

/**
 * A header copy filter that copies all headers.
 *
 * @param name The field name of the HTTP header
 * @param value The field value of the HTTP header
 * @param context The caller-supplied context if any
 * @return A es_http_filter_result_t that controls the iteration
 */
es_http_filter_result_t es_http_header_filter_copy_all(const char *name, const char *value, void *context);

//
// Request
//

/**
 * A HTTP Request URI type (e.g., HTTP, HTTPS)
 */
typedef enum {
  ES_HTTP_URI_ASTERISK = 0, /**< OPTIONS * HTTP/1.1 */
  ES_HTTP_URI_HTTP = 1,     /**< GET http://www.yahoo.com/ HTTP/1.1 */
  ES_HTTP_URI_HTTPS = 2,    /**< POST https://www.yahoo.com/ HTTP/1.1 */
  ES_HTTP_URI_OTHER = 3     /**< POST foo://opaque */
} es_http_request_uri_t;

/**
 * A HTTP Request
 */
typedef struct es_http_request *es_http_request_t;

/**
 * Copy one HTTP request to another, potentially filtering HTTP headers.
 *
 * @param source The source HTTP request (copy from)
 * @param dest The destination HTTP request (copy to)
 * @param allocator The allocator to be used for the destination's copies
 * @param filter A filter callback that can optionally exclude specific headers from the copy
 * @param context An optional context that can be passed to the filter callback
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_copy(const es_http_request_t source, es_http_request_t dest,
                                     es_http_allocator_t allocator, es_http_header_filter filter, void *context);

/**
 * Get the headers of a HTTP request
 *
 * @param request The HTTP request
 * @return The request's headers.  Will never be NULL, but may be an empty list
 */
const es_http_header_list_t es_http_request_headers(const es_http_request_t request);

/**
 * Add a header to a HTTP request, copying the name and value in the process.
 *
 * @param request The HTTP request
 * @param allocator An allocator to be used to allocate memory for the copies
 * @param name The header's field name
 * @param value The header's field value
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_add_header(es_http_request_t request, es_http_allocator_t allocator, const char *name,
                                           const char *value);

/**
 * Set the type of the request's URI
 *
 * @param request The HTTP request
 * @param type The URI type
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_set_uri_type(es_http_request_t request, es_http_request_uri_t type);

/**
 * Get the type of the request's URI
 *
 * @param request The HTTP request
 * @return The request's URI type
 */
es_http_request_uri_t es_http_request_uri_type(const es_http_request_t request);

/**
 * Set the path for a HTTP request
 *
 * @param request The HTTP request
 * @param allocator The allocator to allocate memory for the copy
 * @param path The path to be copied into the request
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_set_uri_path(es_http_request_t request, es_http_allocator_t allocator,
                                             const char *path);

/**
 * Get the path of a HTTP request
 *
 * @param request The HTTP request
 * @return The HTTP request's path or NULL if it has not been set.
 */
const char *es_http_request_uri_path(const es_http_request_t request);

/**
 * Set the query for a HTTP request
 *
 * @param request The HTTP request
 * @param allocator The allocator to allocate memory for the copy
 * @param query The query to be copied into the request
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_set_uri_query(es_http_request_t request, es_http_allocator_t allocator,
                                              const char *query);

/**
 * Get the query of a HTTP request
 *
 * @param request The HTTP request
 * @return The HTTP request's query or NULL if it has not been set.
 */
const char *es_http_request_uri_query(const es_http_request_t request);

/**
 * Set the fragment for a HTTP request
 *
 * @param request The HTTP request
 * @param allocator The allocator to allocate memory for the copy
 * @param fragment The fragment to be copied into the request
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_set_uri_fragment(es_http_request_t request, es_http_allocator_t allocator,
                                                 const char *fragment);

/**
 * Get the fragment of a HTTP request
 *
 * @param request The HTTP request
 * @return The HTTP request's fragment or NULL if it has not been set.
 */
const char *es_http_request_uri_fragment(const es_http_request_t request);

/**
 * Set the host for a HTTP request
 *
 * @param request The HTTP request
 * @param allocator The allocator to allocate memory for the copy
 * @param host The host to be copied into the request
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_set_uri_host(es_http_request_t request, es_http_allocator_t allocator,
                                             const char *host);

/**
 * Get the host of a HTTP request
 *
 * @param request The HTTP request
 * @return The HTTP request's host or NULL if it has not been set.
 */
const char *es_http_request_uri_host(const es_http_request_t request);

/**
 * Set the port for a HTTP request
 *
 * @param request The HTTP request
 * @param port The port (must be a valid unsigned 16 bit integer)
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_set_uri_port(es_http_request_t request, int32_t port);

/**
 * Get the port of a HTTP request
 *
 * @param request The HTTP request
 * @return The HTTP request's port or -1 if it has not been set
 */
int32_t es_http_request_uri_port(const es_http_request_t request);

/**
 * Set the complete URI for a HTTP request with a URI of type ES_HTTP_URI_OTHER
 *
 * @param request The HTTP request
 * @param allocator The allocator to allocate memory for the copy
 * @param host The complete URI to be copied into the request
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_request_set_uri_other(es_http_request_t request, es_http_allocator_t allocator,
                                              const char *other);

/**
 * Get the complete URI of a HTTP request with a URI of type ES_HTTP_URI_OTHER
 *
 * @param request The HTTP request
 * @return The HTTP request's complete URI or NULL if it has not been set.
 */
const char *es_http_request_uri_other(const es_http_request_t request);

//
// Response
//

/**
 * A HTTP Response
 */
typedef struct es_http_response *es_http_response_t;

/**
 * Copy one HTTP response to another, potentially filtering HTTP headers.
 *
 * @param source The source HTTP response (copy from)
 * @param dest The destination HTTP response (copy to)
 * @param allocator The allocator to be used for the destination's copies
 * @param filter A filter callback that can optionally exclude specific headers from the copy
 * @param context An optional context that can be passed to the filter callback
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_response_copy(const es_http_response_t source, es_http_response_t dest,
                                      es_http_allocator_t allocator, es_http_header_filter filter, void *context);

/**
 * Get the headers of a HTTP response
 *
 * @param response The HTTP response
 * @return The request's headers.  Will never be NULL, but may be an empty list
 */
const es_http_header_list_t es_http_response_headers(const es_http_response_t response);

/**
 * Add a header to a HTTP response, copying the name and value in the process.
 *
 * @param response The HTTP response
 * @param allocator An allocator to be used to allocate memory for the copies
 * @param name The header's field name
 * @param value The header's field value
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_response_add_header(es_http_response_t response, es_http_allocator_t allocator,
                                            const char *name, const char *value);

/**
 * Set a HTTP response's status code
 *
 * @param response The HTTP response
 * @param status_code The status code
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_response_set_status_code(es_http_response_t response, int status_code);

/**
 * Get a HTTP response's status code
 *
 * @param response The HTTP response
 * @return The status code, or -1 if not set
 */
int es_http_response_status_code(const es_http_response_t response);

/**
 * Set the reason phrase for a HTTP response
 *
 * @param response The HTTP response
 * @param allocator The allocator to allocate memory for the copy
 * @param fragment The reason phrase to be copied into the request
 * @return ES_HTTP_SUCCESS if successful, another error code otherwise.
 */
es_http_error_t es_http_response_set_reason_phrase(es_http_response_t response, es_http_allocator_t allocator,
                                                   const char *reason_phrase);

/**
 * Get the reason phrase for a HTTP response
 *
 * @param response The HTTP response
 * @return The reason phrase or NULL if not set
 */
const char *es_http_response_reason_phrase(const es_http_response_t response);

/**
 * Get the default reason phrase for a status code
 *
 * @param status_code The status code
 * @param fallback If a reason phrase isn't found, return this instead
 * @return The reason phrase for the status code or the fallback
 */
const char *es_http_response_default_reason_phrase(int status_code, const char *fallback);
}

#endif