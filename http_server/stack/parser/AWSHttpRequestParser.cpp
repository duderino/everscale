/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_PARSER_H
#include <AWSHttpRequestParser.h>
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

#define AWS_PARSING_METHOD (1 << 0)
#define AWS_PARSING_REQUEST_URI (1 << 1)
#define AWS_PARSING_HTTP_VERSION (1 << 2)
#define AWS_PARSE_COMPLETE (1 << 3)

AWSHttpRequestParser::AWSHttpRequestParser(ESFBuffer *workingBuffer, ESFDiscardAllocator *allocator) :
    AWSHttpMessageParser(workingBuffer, allocator),
    _requestState(0x00),
    _requestUriParser(workingBuffer, allocator)
{
}

AWSHttpRequestParser::~AWSHttpRequestParser()
{
}

void AWSHttpRequestParser::reset()
{
    AWSHttpMessageParser::reset();

    _requestState = 0x00;
    _requestUriParser.reset();
}

ESFError AWSHttpRequestParser::parseStartLine(ESFBuffer *inputBuffer, AWSHttpMessage *message)
{
    // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF

    if (AWS_PARSE_COMPLETE & _requestState)
    {
        return ESF_INVALID_STATE;
    }

    if (0x00 == _requestState)
    {
        _requestState = AWS_PARSING_METHOD;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    AWSHttpRequest *request = (AWSHttpRequest *) message;

    ESFError error = ESF_SUCCESS;

    if (AWS_PARSING_METHOD & _requestState)
    {
        error = parseMethod(inputBuffer, request);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _requestState &= ~AWS_PARSING_METHOD;
        _requestState |= AWS_PARSING_REQUEST_URI;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    if (AWS_PARSING_REQUEST_URI & _requestState)
    {
        error = _requestUriParser.parse(inputBuffer, request->getRequestUri());

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _requestState &= ~AWS_PARSING_REQUEST_URI;
        _requestState |= AWS_PARSING_HTTP_VERSION;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    if (AWS_PARSING_HTTP_VERSION & _requestState)
    {
        error = parseVersion(inputBuffer, request, false);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _requestState &= ~AWS_PARSING_HTTP_VERSION;
        _requestState |= AWS_PARSE_COMPLETE;

        inputBuffer->readMark();
        _workingBuffer->clear();

        return ESF_SUCCESS;
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpRequestParser::parseMethod(ESFBuffer *inputBuffer, AWSHttpRequest *request)
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

    ESF_ASSERT(AWS_PARSING_METHOD & _requestState);

    unsigned char octet;

    while (true)
    {
        if (false == inputBuffer->isReadable())
        {
            return ESF_AGAIN;
        }

        if (false == _workingBuffer->isWritable())
        {
            return ESF_OVERFLOW;
        }

        octet = inputBuffer->getNext();

        if (AWSHttpUtil::IsSpace(octet))
        {
            request->setMethod(_workingBuffer->duplicate(_allocator));

            return 0 == request->getMethod() ? ESF_OUT_OF_MEMORY : ESF_SUCCESS;
        }

        if (AWSHttpUtil::IsToken(octet))
        {
            _workingBuffer->putNext(octet);
            continue;
        }

        return AWS_HTTP_BAD_REQUEST_METHOD;
    }
}

bool AWSHttpRequestParser::isBodyNotAllowed(AWSHttpMessage *message)
{
    // A message-body MUST NOT be included in
    // a request if the specification of the request method (section 5.1.1)
    // does not allow sending an entity-body in requests. A server SHOULD
    // read and forward a message-body on any request; if the request method
    // does not include defined semantics for an entity-body, then the
    // message-body SHOULD be ignored when handling the request.

    AWSHttpRequest *request = (AWSHttpRequest *) message;

    if (0 == strcasecmp("GET", (const char *) request->getMethod()) ||
        0 == strcasecmp("DELETE", (const char *) request->getMethod()))
    {
        return true;
    }

    return false;
}



