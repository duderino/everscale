/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H
#define AWS_HTTP_ECHO_CLIENT_REQUEST_BUILDER_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef AWS_HTTP_CLIENT_TRANSACTION_H
#include <AWSHttpClientTransaction.h>
#endif

extern ESFError AWSHttpEchoClientRequestBuilder(
    const char *host, int port, const char *absPath, const char *method,
    const char *contentType, AWSHttpClientTransaction *transaction);

#endif
