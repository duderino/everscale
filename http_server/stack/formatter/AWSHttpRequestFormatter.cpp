/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_FORMATTER_H
#include <AWSHttpRequestFormatter.h>
#endif

#ifndef AWS_HTTP_UTIL_H
#include <AWSHttpUtil.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef AWS_HTTP_ERROR_H
#include <AWSHttpError.h>
#endif

#define AWS_FORMATTING_METHOD (1 << 0)
#define AWS_FORMATTING_REQUEST_URI (1 << 1)
#define AWS_FORMATTING_HTTP_VERSION (1 << 2)
#define AWS_FORMAT_COMPLETE (1 << 3)

AWSHttpRequestFormatter::AWSHttpRequestFormatter() :
    _requestState(0x00),
    _requestUriFormatter()
{
}

AWSHttpRequestFormatter::~AWSHttpRequestFormatter()
{
}

void AWSHttpRequestFormatter::reset()
{
    AWSHttpMessageFormatter::reset();

    _requestState = 0x00;
    _requestUriFormatter.reset();
}

ESFError AWSHttpRequestFormatter::formatStartLine(ESFBuffer *outputBuffer, const AWSHttpMessage *message)
{
    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

    if (AWS_FORMAT_COMPLETE & _requestState)
    {
        return ESF_INVALID_STATE;
    }

    if (0x00 == _requestState)
    {
        AWSHttpUtil::Start(&_requestState, outputBuffer, AWS_FORMATTING_METHOD);
    }

    AWSHttpRequest *request = (AWSHttpRequest *) message;

    ESFError error = ESF_SUCCESS;

    if (AWS_FORMATTING_METHOD & _requestState)
    {
        error = formatMethod(outputBuffer, request);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        AWSHttpUtil::Transition(&_requestState, outputBuffer, AWS_FORMATTING_METHOD, AWS_FORMATTING_REQUEST_URI);
    }

    if (AWS_FORMATTING_REQUEST_URI & _requestState)
    {
        error = _requestUriFormatter.format(outputBuffer, request->getRequestUri());

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        AWSHttpUtil::Transition(&_requestState, outputBuffer, AWS_FORMATTING_REQUEST_URI, AWS_FORMATTING_HTTP_VERSION);
    }

    if (AWS_FORMATTING_HTTP_VERSION & _requestState)
    {
        error = formatVersion(outputBuffer, request, true);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        return AWSHttpUtil::Transition(&_requestState, outputBuffer, AWS_FORMATTING_HTTP_VERSION, AWS_FORMAT_COMPLETE);
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpRequestFormatter::formatMethod(ESFBuffer *outputBuffer, const AWSHttpRequest *request)
{
    // Method                = "OPTIONS"                ; Section 9.2
    //                       | "GET"                    ; Section 9.3
    //                       | "HEAD"                   ; Section 9.4
    //                       | "POST"                   ; Section 9.5
    //                       | "PUT"                    ; Section 9.6
    //                       | "DELETE"                 ; Section 9.7
    //                       | "TRACE"                  ; Section 9.8
    //                       | "CONNECT"                ; Section 9.9
    //                       | extension-method
    // extension-method = token

    ESF_ASSERT(AWS_FORMATTING_METHOD & _requestState);

    if (0 == request->getMethod() || 0 == request->getMethod()[0])
    {
        return AWS_HTTP_BAD_REQUEST_METHOD;
    }

    for (const unsigned char *p = request->getMethod(); *p; ++p)
    {
        if (false == outputBuffer->isWritable())
        {
            return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
        }

        if (AWSHttpUtil::IsToken(*p))
        {
            outputBuffer->putNext(*p);
            continue;
        }

        return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REQUEST_METHOD);
    }

    if (false == outputBuffer->isWritable())
    {
        return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext(' ');

    return ESF_SUCCESS;
}
