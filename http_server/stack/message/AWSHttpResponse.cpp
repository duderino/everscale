/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_RESPONSE_H
#include <AWSHttpResponse.h>
#endif

AWSHttpResponse::AWSHttpResponse() :
    AWSHttpMessage(),
    _statusCode(0),
    _reasonPhrase(0)
{
}

AWSHttpResponse::~AWSHttpResponse()
{
}

void AWSHttpResponse::reset()
{
    AWSHttpMessage::reset();
    _statusCode = 0;
    _reasonPhrase = 0;
}

