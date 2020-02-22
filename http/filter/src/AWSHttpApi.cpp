/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_API_H
#include <AWSHttpApi.h>
#endif

#ifndef AWS_DEFAULT_HTTP_PORT
#define AWS_DEFAULT_HTTP_PORT 80
#endif

#ifndef AWS_DEFAULT_HTTPS_PORT
#define AWS_DEFAULT_HTTPS_PORT 443
#endif

#ifndef AWS_DEFAULT_THREADS
#define AWS_DEFAULT_THREADS 4
#endif

// TODO Support HTTPS
// TODO Get server callbacks into AWSHttpStack

AWSHttpApi::AWSHttpApi(aws_http_server *server,
                       aws_http_server_config *server_config,
                       aws_http_log_level log_level, void *log_context,
                       aws_http_logger logger)
    : _logger((ESFLogger::Severity)log_level, logger, log_context),
      _stack(server_config ? server_config->http_port : AWS_DEFAULT_HTTP_PORT,
             server_config ? server_config->threads : AWS_DEFAULT_THREADS,
             &_logger) {}

AWSHttpApi::~AWSHttpApi() {}

AWSHttpRequest *AWSHttpApi::createRequest() { todo }

void AWSHttpApi::destroyRequest(AWSHttpRequest *request){todo}

AWSHttpResponse *AWSHttpApi::createResponse() {
  todo
}

void AWSHttpApi::destroyResponse(AWSHttpResponse *AWSHttpApi::response) { todo }
