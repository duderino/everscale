/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_H
#include <aws_http.h>
#endif

#ifndef AWS_HTTP_API_H
#include <AWSHttpApi.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

static int aws_error_to_errno(ESFError error);

aws_http_stack aws_http_stack_create(aws_http_server *server,
                                     aws_http_server_config *server_config,
                                     aws_http_log_level log_level,
                                     void *log_context,
                                     aws_http_logger logger) {
  // TODO make sure http stack can be used with NULL server arguments
  AWSHttpApi *api =
      new AWSHttpApi(server, server_config, log_level, log_context, logger);

  if (0 == api) {
    errno = ENOMEM;

    return 0;
  }

  ESFError error = api->initialize();

  if (ESF_SUCCESS != error) {
    delete api;

    errno = aws_error_to_errno(error);

    return 0;
  }

  return (aws_http_stack *)api;
}

int aws_http_stack_start(aws_http_stack stack) {
  if (!stack) {
    errno = EFAULT;
    return -1;
  }

  AWSHttpApi *api = (AWSHttpApi *)stack;

  ESFError error = api->start();

  if (ESF_SUCCESS != error) {
    errno = aws_error_to_errno(error);

    return -1;
  }

  return 0;
}

int aws_http_stack_stop(aws_http_stack stack) {
  if (!stack) {
    errno = EFAULT;
    return -1;
  }

  AWSHttpApi *api = (AWSHttpApi *)stack;

  ESFError error = api->stop();

  if (ESF_SUCCESS != error) {
    errno = aws_error_to_errno(error);

    return -1;
  }

  return 0;
}

int aws_http_stack_destroy(aws_http_stack stack) {
  if (!stack) {
    errno = EFAULT;
    return -1;
  }

  AWSHttpApi *api = (AWSHttpApi *)stack;

  ESFError error = api->destroy();

  if (ESF_SUCCESS != error) {
    errno = aws_error_to_errno(error);

    return -1;
  }

  delete api;

  return 0;
}

void aws_http_stack_set_log_level(aws_http_stack stack,
                                  aws_http_log_level log_level) {
  if (!stack) {
    errno = EFAULT;
    return -1;
  }

  ((AWSHttpApi *)stack)->setLogLevel((ESFLogger::Severity)log_level);
}

aws_http_request aws_http_stack_create_request(aws_http_stack stack) {
  if (!stack) {
    errno = EFAULT;
    return -1;
  }

  AWSHttpApi *api = (AWSHttpApi *)stack;

  AWSHttpRequest *request = api->createRequest();

  if (!request) {
    errno = ENOMEM;

    return 0;
  }

  return (aws_http_request)request;
}

int aws_http_stack_send_request(aws_http_stack stack,
                                const struct sockaddr_in *address,
                                aws_http_request request, void *client_context,
                                aws_http_client *client,
                                aws_http_client_config *client_config) {
  if (!stack || !request || !client || !client_config) {
    errno = EFAULT;
    return -1;
  }

  AWSHttpApi *api = (AWSHttpApi *)stack;
  AWSHttpRequest *request = (AWSHttpRequest *)request;

  char dottedIp[16];
  unsigned short port;

  if (!address) {
    // todo guess from request uri and host header.
  } else {
    inet_ntop(AF_INET, &address->sin_addr, dottedIp, sizeof(dottedIp));
    port = ntohs(address->sin_port);
  }

  ESFSocketAddress socketAddress(dottedIp, port, ESFSocketAddress::TCP);

  // TODO get callback into client socket

  ESFError error = api->sendRequest(request, socketAddress);

  if (ESF_SUCCESS != error) {
    errno = aws_error_to_errno(error);

    return -1;
  }

  return 0;
}

void aws_http_stack_destroy_request(aws_http_stack stack,
                                    aws_http_request request) {
  if (!stack || !request) {
    return;
  }

  ((AWSHttpApi *)stack)->destroyRequest((AWSHttpRequest *)request);
}

const unsigned char *aws_http_header_get_field_name(
    aws_http_stack stack, const aws_http_header header) {
  if (!header) {
    errno = EFAULT;
    return 0;
  }

  return ((AWSHttpHeader *)header)->getFieldName();
}

extern int aws_http_header_set_field_name(aws_http_stack stack,
                                          aws_http_header header,
                                          const unsigned char *field_name);
extern const unsigned char *aws_http_header_get_field_value(
    aws_http_stack stack, aws_http_header header);
extern int aws_http_header_set_field_value(aws_http_stack stack,
                                           aws_http_header header,
                                           const unsigned char *field_value);

extern aws_http_header aws_http_message_get_first_header(
    aws_http_stack stack, aws_http_message message);
extern aws_http_header aws_http_message_get_next_header(
    aws_http_stack stack, aws_http_message message, aws_http_header header);
extern int aws_http_message_has_next_header(aws_http_stack stack,
                                            aws_http_message message,
                                            aws_http_header header);
extern int aws_http_message_add_header(aws_http_stack stack,
                                       aws_http_message message,
                                       const unsigned char *field_name,
                                       const unsigned char *field_value);
extern float aws_http_message_get_version(aws_http_stack stack,
                                          aws_http_message message);
extern int aws_http_message_set_version(aws_http_connection connection,
                                        aws_http_message message,
                                        float version);
extern void aws_http_message_set_has_body(aws_http_connection connection,
                                          aws_http_message message,
                                          int has_body);
extern int aws_http_message_has_body(aws_http_connection connection,
                                     aws_http_message message);

extern void aws_http_request_reset(aws_http_connection connection,
                                   aws_http_request request);
extern const unsigned char *aws_http_request_get_method(
    aws_http_connection connection, aws_http_request request);
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

static int aws_error_to_errno(ESFError error) {
  switch (error) {
    case ESF_SUCCESS:

      return 0;

    case ESF_OTHER_ERROR:

      return EPERM;

    case ESF_OPERATION_NOT_SUPPORTED:

      return ENOTSUP;

    case ESF_NULL_POINTER:

      return EFAULT;

    case ESF_UNIQUENESS_VIOLATION:

      return EEXIST;

    case ESF_INVALID_ARGUMENT:

      return EINVAL;

    case ESF_OUT_OF_MEMORY:

      return ENOMEM;

    case ESF_NOT_INITIALIZED:

      return EPERM;

    case ESF_AGAIN:

      return EGAIN;

    case ESF_INTR:

      return EINTR;

    case ESF_INPROGRESS:

      return EINPROGRESS;

    case ESF_TIMEOUT:

      return ETIMEDOUT;

    case ESF_ARGUMENT_TOO_SHORT:

      return EINVAL;

    case ESF_RESULT_TRUNCATED:

      return EOVERFLOW;

    case ESF_INVALID_STATE:

      return EPERM;

    case ESF_NOT_OWNER:

      return EACCES;

    case ESF_IN_USE:

      return EBUSY;

    case ESF_CANNOT_FIND:

      return ENOATTR;

    case ESF_INVALID_ITERATOR:

      return EINVAL;

    case ESF_OVERFLOW:

      return EOVERFLOW;

    case ESF_CANNOT_CONVERT:
    case ESF_ILLEGAL_ENCODING:
    case ESF_UNSUPPORTED_CHARSET:

      return EILSEQ;

    case ESF_OUT_OF_BOUNDS:

      return EINVAL;

    case ESF_SHUTDOWN:

      return EPERM;

    default:

      return error;
  }
}
