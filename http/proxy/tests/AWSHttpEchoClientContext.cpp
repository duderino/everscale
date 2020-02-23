/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_ECHO_CLIENT_CONTEXT_H
#include <AWSHttpEchoClientContext.h>
#endif

AWSHttpEchoClientContext::AWSHttpEchoClientContext(
    unsigned int remainingIterations)
    : _bytesSent(0U), _iterations(remainingIterations) {}

AWSHttpEchoClientContext::~AWSHttpEchoClientContext() {}
