/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_H
#define AWS_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void *aws_http_header;
typedef void *aws_http_message;
typedef void *aws_http_request;
typedef void *aws_http_request_uri;
typedef void *aws_http_response;
typedef void *aws_http_stack;
typedef void *aws_http_allocator;
typedef void *aws_http_transaction;

/*
 * Server Module API
 */

typedef enum {
  AWS_HTTP_SERVER_HANDLER_CONTINUE = 0, /**< Continue processing the request */
  AWS_HTTP_SERVER_HANDLER_CLOSE =
      1, /**< Immediately close the socket / abort the request */
  AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE =
      2 /**< Immediately send the response / finish the request */
} aws_http_server_handler_result;

/** Accept a new connection.
 *
 * @param stack The stack that accepted the connection
 * @param address The IP address and port of the client
 * @return a result code
 */
typedef aws_http_server_handler_result (
    *aws_http_server_handler_accept_connection)(
    const struct sockaddr_in *address);

/**
 * Handle the beginning of a transaction.  This will be called 1+ times after a
 * connection has been accepted (>1 when connections are reused).  Put your
 * initialization code here.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc
 * @return a result code
 */
typedef aws_http_server_handler_result (
    *aws_http_server_handler_begin_transaction)(
    aws_http_transaction transaction);

/**
 * Process a request's HTTP headers.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @return a result code
 */
typedef aws_http_server_handler_result (
    *aws_http_server_handler_receive_request_headers)(
    aws_http_transaction transaction);

/**
 * Incrementally process a request's body.  This will be called 1+ times as the
 * request's HTTP body is received.  The body is finished when a chunk_size of 0
 * is passed to the callback.  If there is no body, this callback will still be
 * called once with a chunk_size of 0.  The HTTP response object must be
 * populated before the chunk_size 0 call returns.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @param chunk A buffer to drain
 * @param chunkSize The size of the buffer to drain, or 0 if the body is
 * finished.
 * @return a result code
 */
typedef aws_http_server_handler_result (
    *aws_http_server_handler_receive_request_body)(
    aws_http_transaction transaction, unsigned const char *chunk,
    unsigned int chunkSize);

/**
 * Request a buffer of up to n bytes to fill with response body data.  The stack
 * will subsequently call fillResponseChunk with the actual size of the buffer
 * available.  The actual size of the buffer may be less than the requested
 * size.
 *
 * A good implementation will always return the known size of remaining body
 * data.  This function may be called multiple times before fillResponseChunk
 * actually writes the data to the buffer if there is insufficient space in the
 * underlying tcp buffers.  Don't 'deduct' the amount requested from the
 * remaining amount until fillResponseChunk is called.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @return The buffer size requested.  Returning 0 ends the body.  Returning -1
 * or less immediately closes the connection
 */
typedef int (*aws_http_server_handler_reserve_response_chunk)(
    aws_http_transaction transaction);

/**
 * Fill a response body chunk with data.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @param chunk A buffer to fill
 * @param chunkSize The size of the buffer to fill.  This may be less than the
 * size requested by the requestResponseChunk method.
 */
typedef void (*aws_http_server_handler_fill_response_chunk)(
    aws_http_transaction transaction, unsigned char *chunk,
    unsigned int chunkSize);

typedef enum {
  AWS_HTTP_SERVER_HANDLER_BEGIN = 0, /**< Connection accepted or reused */
  AWS_HTTP_SERVER_HANDLER_RECV_REQUEST_HEADERS =
      1, /**< Trying to parse request's http headers */
  AWS_HTTP_SERVER_HANDLER_RECV_REQUEST_BODY =
      2, /**< Trying to read request's http body */
  AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE_HEADERS =
      3, /**< Trying to format and send response's http headers */
  AWS_HTTP_SERVER_HANDLER_SEND_RESPONSE_BODY =
      4,                          /**< Trying to send response's http body */
  AWS_HTTP_SERVER_HANDLER_END = 5 /**< Response sent / transaction completed */
} aws_http_server_handler_state;

/**
 * Handle the end of a transaction.  This is called regardless of the
 * transaction's success or failure.  Put your cleanup code here.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc
 * @param state The state at which the transaction ended.
 * AWS_HTTP_SERVER_HANDLER_END means the transaction was successfully completed,
 * any other state indicates error - reason will be in the server logs.
 */
typedef void (*aws_http_server_handler_end_transaction)(
    aws_http_transaction transaction, aws_http_server_handler_state state);

typedef struct {
  aws_http_server_handler_accept_connection accept_connection;
  aws_http_server_handler_begin_transaction begin_transaction;
  aws_http_server_handler_receive_request_headers receive_request_headers;
  aws_http_server_handler_receive_request_body receive_request_body;
  aws_http_server_handler_reserve_response_chunk reserve_response_chunk;
  aws_http_server_handler_fill_response_chunk fill_response_chunk;
  aws_http_server_handler_end_transaction end_transaction;
} aws_http_server;

typedef struct {
  unsigned int http_port;
  unsigned int threads;
  unsigned int recv_timeout_msec;
  unsigned int send_timeout_msec;
} aws_http_server_config;

typedef enum {
  AWS_HTTP_LOG_NONE = 0,       /**< No logging */
  AWS_HTTP_LOG_EMERGENCYY = 1, /**< System-wide non-recoverable error. */
  AWS_HTTP_LOG_ALERT = 2,    /**< System-wide non-recoverable error imminent. */
  AWS_HTTP_LOG_CRITICAL = 3, /**< System-wide potentially recoverable error. */
  AWS_HTTP_LOG_ERROR = 4,    /**< Localized non-recoverable error. */
  AWS_HTTP_LOG_WARNING = 5,  /**< Localized potentially recoverable error. */
  AWS_HTTP_LOG_NOTICE = 6,   /**< Important non-error event.  */
  AWS_HTTP_LOG_INFO = 7,     /**< Non-error event. */
  AWS_HTTP_LOG_DEBUG = 8     /**< Debugging event. */
} aws_http_log_level;

typedef int (*aws_http_logger)(void *log_context, aws_http_log_level level,
                               const char *file, int line, const char *format,
                               va_list ap);

typedef struct {
  unsigned int connect_timeout_msec;
  unsigned int recv_timeout_msec;
  unsigned int send_timeout_msec;
} aws_http_client_config;

extern aws_http_stack aws_http_stack_create(
    aws_http_server *server, const aws_http_server_config *server_config,
    const aws_http_client_config *client_config, aws_http_log_level log_level,
    void *log_context, aws_http_logger logger);
extern int aws_http_stack_start(aws_http_stack stack);
extern int aws_http_stack_stop(aws_http_stack stack);
extern int aws_http_stack_destroy(aws_http_stack stack);
extern void aws_http_stack_set_log_level(aws_http_stack stack,
                                         aws_http_log_level log_level);

typedef enum {
  AWS_HTTP_CLIENT_HANDLER_CONTINUE = 0, /**< Continue processing the request */
  AWS_HTTP_CLIENT_HANDLER_CLOSE =
      1 /**< Immediately close the socket / abort the request */
} aws_http_client_handler_result;

/**
 * Request a buffer of up to n bytes to fill with request body data.  The stack
 * will subsequently call fillRequestChunk with the actual size of the buffer
 * available.  The actual size of the buffer may be less than the requested
 * size.
 *
 * A good implementation will always return the known size of remaining body
 * data.  This function may be called multiple times before fillRequestChunk
 * actually writes the data to the buffer if there is insufficient space in the
 * underlying tcp buffers.  Don't 'deduct' the amount requested from the
 * remaining amount until fillRequestChunk is called.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @return The buffer size requested.  Returning 0 ends the body.  Returning -1
 * or less immediately closes the connection
 */
typedef int (*aws_http_client_handler_reserve_request_chunk)(
    aws_http_transaction transaction);

/**
 * Fill a request body chunk with data.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @param chunk A buffer to fill
 * @param chunkSize The size of the buffer to fill.  This may be less than the
 * size requested by the requestRequestChunk method.
 */
typedef void (*aws_http_client_handler_fill_request_chunk)(
    aws_http_transaction transaction, unsigned char *chunk,
    unsigned int chunkSize);

/**
 * Process a request's HTTP headers.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @return a result code
 */
typedef aws_http_client_handler_result (
    *aws_http_client_handler_receive_response_headers)(
    aws_http_transaction transaction);

/**
 * Incrementally process a response body.  This will be called 1+ times as the
 * HTTP response body is received.  The body is finished when a chunk_size of 0
 * is passed to the callback.  If there is no body, this callback will still be
 * called once with a chunk_size of 0.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc.
 * @param chunk A buffer to drain
 * @param chunkSize The size of the buffer to drain, or 0 if the body is
 * finished.
 * @return a result code
 */
typedef aws_http_client_handler_result (
    *aws_http_client_handler_receive_response_body)(
    aws_http_transaction transaction, unsigned const char *chunk,
    unsigned int chunkSize);
typedef enum {
  AWS_HTTP_CLIENT_HANDLER_BEGIN =
      0, /**< Transaction starting - nothing happend yet */
  AWS_HTTP_CLIENT_HANDLER_RESOLVE = 1, /**< Trying to resolve client address */
  AWS_HTTP_CLIENT_HANDLER_CONNECT = 2, /**< Trying to connect to client */
  AWS_HTTP_CLIENT_HANDLER_SEND_REQUEST_HEADERS =
      3, /**< Trying to send request headers */
  AWS_HTTP_CLIENT_HANDLER_SEND_REQUEST_BODY =
      4, /**< Trying to send request body */
  AWS_HTTP_CLIENT_HANDLER_RECV_RESPONSE_HEADERS =
      5, /**< Trying to receive/parse response headers */
  AWS_HTTP_CLIENT_HANDLER_RECV_RESPONSE_BODY =
      6, /**< Trying to receive response body */
  AWS_HTTP_CLIENT_HANDLER_END =
      7 /**< Response received / transaction completed */
} aws_http_client_handler_state;

/**
 * Handle the end of a transaction.  This is called regardless of the
 * transaction's success or failure.  Put your cleanup code here.
 *
 * @param transaction The http transaction - contains request and response
 * objects, etc
 * @param state The state at which the transaction ended.
 * AWS_HTTP_CLIENT_HANDLER_END means the transaction was successfully completed,
 * any other state indicates error - reason will be in the server logs.
 */
typedef void (*aws_http_client_handler_end_transaction)(
    aws_http_transaction transaction, aws_http_client_handler_state state);

typedef struct {
  aws_http_client_handler_reserve_request_chunk reserve_request_chunk;
  aws_http_client_handler_fill_request_chunk fill_request_chunk;
  aws_http_client_handler_receive_response_headers receive_response_headers;
  aws_http_client_handler_receive_response_body receive_response_body;
  aws_http_client_handler_end_transaction end_transaction;
} aws_http_client;

/**
 * Create a new client transaction
 *
 * @param stack The stack that will execute the client transaction
 * @param client The client callback functions
 * @param client_context app-specific client context
 * @return a new client transaction if successful, null otherwise
 */
extern aws_http_transaction aws_http_stack_create_client_transaction(
    aws_http_stack stack, aws_http_client *client, void *client_context);

/**
 * Execute the client transaction.  If this method returns ESF_SUCCESS, then the
 * transaction will be cleaned up automatically after it finishes.  If this
 * method returns anything else then the caller should clean it up with
 * destroyClientTransaction
 *
 * @param stack The stack that created the transaction
 * @param transaction The transaction
 * @return 0 if the transaction was successfully started, -1 otherwise.  If -1
 * is returned, errno will be set and the caller should cleanup the transaction
 * with aws_http_stack_destroy_client_transaction
 */
extern int aws_http_stack_execute_client_transaction(
    aws_http_stack stack, aws_http_transaction transaction);

/**
 * Cleanup the client transaction.  Note that this will not free any
 * app-specific context.  Call this only if executeClientTransaction doesn't
 * return 0.
 *
 * @param stack The stack that created the transaction
 * @param transaction The transaction
 */
extern void aws_http_stack_destroy_transaction(aws_http_stack stack,
                                               aws_http_transaction);
extern void aws_http_stack_destroy_request(aws_http_stack stack,
                                           aws_http_request request);

extern const unsigned char *aws_http_header_get_field_name(
    const aws_http_header header);
extern int aws_http_header_set_field_name(aws_http_message message,
                                          aws_http_header header,
                                          const unsigned char *field_name);
extern const unsigned char *aws_http_header_get_field_value(
    const aws_http_header header);
extern int aws_http_header_set_field_value(aws_http_message message,
                                           aws_http_header header,
                                           const unsigned char *field_value);

extern aws_http_header aws_http_message_get_first_header(
    aws_http_message message);
extern aws_http_header aws_http_message_get_last_header(aws_http_message);
extern aws_http_header aws_http_message_find_first_header(
    aws_http_message, const unsigned char *field_name);
extern aws_http_header aws_http_message_find_last_header(
    aws_http_message, const unsigned char *field_name);
extern aws_http_header aws_http_message_get_next_header(aws_http_header header);
extern aws_http_header aws_http_message_get_previous_header(
    aws_http_header header);
extern int aws_http_message_has_next_header(aws_http_header header);
extern int aws_http_message_has_previous_header(aws_http_header header);
extern int aws_http_message_append_header(aws_http_message message,
                                          const unsigned char *field_name,
                                          const unsigned char *field_value,
                                          aws_http_transaction transaction);
extern int aws_http_message_prepend_header(aws_http_message message,
                                           const unsigned char *field_name,
                                           const unsigned char *field_value,
                                           aws_http_transaction transaction);

extern float aws_http_message_get_version(const aws_http_message message);
extern int aws_http_message_set_version(aws_http_message message,
                                        float version);

extern void aws_http_message_set_has_body(aws_http_message message,
                                          int has_body);
extern int aws_http_message_has_body(const aws_http_message message);

extern const unsigned char *aws_http_request_get_method(
    const aws_http_request request);
extern int aws_http_request_set_method(aws_http_connection connection,
                                       aws_http_request request,
                                       const unsigned char *method);
extern aws_http_request_uri *aws_http_request_get_request_uri(
    aws_http_connection connection, aws_http_request request);

extern void aws_http_response_reset(aws_http_connection connection,
                                    aws_http_response response);
extern int aws_http_response_set_status_code(aws_http_connection connection,
                                             aws_http_response response,
                                             int status_code);
extern int aws_http_response_get_status_code(aws_http_connection connection,
                                             aws_http_response response);
extern int aws_http_response_set_reason_phrase(
    aws_http_connection connection, aws_http_response response,
    const unsigned char *reason_phrase);
extern const unsigned char *aws_http_response_get_reason_phrase(
    aws_http_connection connection, aws_http_response response);

extern void aws_http_request_uri_reset(aws_http_connection connection,
                                       aws_http_request_uri request_uri);
extern void aws_http_request_uri_set_is_secure(aws_http_connection connection,
                                               int is_secure);
extern int aws_http_request_uri_is_secure(aws_http_connection connection,
                                          aws_http_request_uri request_uri);
extern const unsigned char *aws_http_request_uri_get_abs_path(
    aws_http_connection connection, aws_http_request_uri request_uri);
extern int aws_http_request_uri_set_abs_path(aws_http_connection connection,
                                             aws_http_request_uri request_uri,
                                             const unsigned char *abs_path);
extern const unsigned char *aws_http_request_uri_get_query(
    aws_http_connection connection, aws_http_request_uri request_uri);
extern int aws_http_request_uri_set_query(aws_http_connection connection,
                                          aws_http_request_uri request_uri,
                                          const unsigned char *query);
extern const unsigned char *aws_http_request_uri_get_host(
    aws_http_connection connection, aws_http_request_uri request_uri);
extern int aws_http_request_uri_set_host(aws_http_connection connection,
                                         aws_http_request_uri request_uri,
                                         const unsigned char *host);
extern int aws_http_request_uri_get_port(aws_http_connection connection,
                                         aws_http_request_uri request_uri);
extern int aws_http_request_uri_set_port(aws_http_connection connection,
                                         aws_http_request_uri request_uri,
                                         int port);
extern const unsigned char *aws_http_request_uri_get_fragment(
    aws_http_connection connection, aws_http_request_uri request_uri);
extern int aws_http_request_uri_set_fragment(aws_http_connection connection,
                                             aws_http_request_uri request_uri,
                                             const unsigned char *fragment);
extern int aws_http_request_uri_compare(
    const aws_http_request_uri left_request_uri,
    const aws_http_request_uri right_request_uri);

extern aws_http_allocator aws_http_connection_get_allocator(
    aws_http_connection connection);
extern int aws_http_connection_is_secure(const aws_http_connection connection);
extern int aws_http_connection_get_remote_port(
    const aws_http_connection connection);
extern struct in_addr aws_http_connection_get_remote_address(
    const aws_http_connection connection);
extern int aws_http_connection_get_local_port(
    const aws_http_connection connection);
extern struct in_addr aws_http_connection_get_local_address(
    const aws_http_connection connection);
extern aws_http_stack aws_http_connection_get_http_stack(
    const aws_http_connection connection);

extern void *aws_http_allocator_allocate(aws_http_allocator allocator,
                                         size_t size);
extern void aws_http_allocator_deallocate(aws_http_allocator allocator,
                                          void *block);

#ifdef __cplusplus
};
#endif

#endif
