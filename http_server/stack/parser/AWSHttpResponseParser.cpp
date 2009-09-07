/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_RESPONSE_PARSER_H
#include <AWSHttpResponseParser.h>
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

#define AWS_PARSING_VERSION (1 << 0)
#define AWS_PARSING_STATUS_CODE (1 << 1)
#define AWS_PARSING_REASON_PHRASE (1 << 2)
#define AWS_PARSE_COMPLETE (1 << 3)

AWSHttpResponseParser::AWSHttpResponseParser(ESFBuffer *workingBuffer, ESFDiscardAllocator *allocator) :
    AWSHttpMessageParser(workingBuffer, allocator),
    _responseState(0x00)
{
}

AWSHttpResponseParser::~AWSHttpResponseParser()
{
}

void AWSHttpResponseParser::reset()
{
    AWSHttpMessageParser::reset();

    _responseState = 0x00;
}

ESFError AWSHttpResponseParser::parseStartLine(ESFBuffer *inputBuffer, AWSHttpMessage *message)
{
    // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

    if (AWS_PARSE_COMPLETE & _responseState)
    {
        return ESF_INVALID_STATE;
    }

    if (0x00 == _responseState)
    {
        _responseState = AWS_PARSING_VERSION;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    AWSHttpResponse *response = (AWSHttpResponse *) message;

    ESFError error = ESF_SUCCESS;

    if (AWS_PARSING_VERSION & _responseState)
    {
        error = parseVersion(inputBuffer, response, true);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _responseState &= ~AWS_PARSING_VERSION;
        _responseState |= AWS_PARSING_STATUS_CODE;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    if (AWS_PARSING_STATUS_CODE & _responseState)
    {
        error = parseStatusCode(inputBuffer, response);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _responseState &= ~AWS_PARSING_STATUS_CODE;
        _responseState |= AWS_PARSING_REASON_PHRASE;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    if (AWS_PARSING_REASON_PHRASE & _responseState)
    {
        error = parseReasonPhrase(inputBuffer, response);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _responseState &= ~AWS_PARSING_REASON_PHRASE;
        _responseState |= AWS_PARSE_COMPLETE;

        inputBuffer->readMark();
        _workingBuffer->clear();

        return ESF_SUCCESS;
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpResponseParser::parseStatusCode(ESFBuffer *inputBuffer, AWSHttpResponse *response)
{
    // Status-Code    = 3DIGIT

    ESF_ASSERT(AWS_PARSING_STATUS_CODE & _responseState);

    // Clients SHOULD be tolerant in parsing the Status-Line and servers
    // tolerant when parsing the Request-Line. In particular, they SHOULD
    // accept any amount of SP or HT characters between fields, even though
    // only a single SP is required.

    AWSHttpUtil::SkipSpaces(inputBuffer);

    if (4 > inputBuffer->getReadable())
    {
        return ESF_AGAIN;
    }

    int statusCode = 0;
    unsigned char octet;

    for (int i = 0; i < 3; ++i)
    {
        ESF_ASSERT(inputBuffer->isReadable());

        octet = inputBuffer->getNext();

        if (false == AWSHttpUtil::IsDigit(octet))
        {
            return AWS_HTTP_BAD_STATUS_CODE;
        }

        statusCode = (statusCode * 10) + (octet - '0');
    }

    ESF_ASSERT(inputBuffer->isReadable());

    if (false == AWSHttpUtil::IsSpace(inputBuffer->getNext()))
    {
        return AWS_HTTP_BAD_STATUS_CODE;
    }

    response->setStatusCode(statusCode);

    return ESF_SUCCESS;
}


ESFError AWSHttpResponseParser::parseReasonPhrase(ESFBuffer *inputBuffer, AWSHttpResponse *response)
{
    // Reason-Phrase  = *<TEXT, excluding CR, LF>

    ESF_ASSERT(AWS_PARSING_REASON_PHRASE & _responseState);

    ESFError error;
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

        if (AWSHttpUtil::IsLWS(octet))
        {
            inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);

            error = AWSHttpUtil::SkipLWS(inputBuffer);

            switch (error)
            {
                case ESF_SUCCESS:

                    // newline encountered - save reason phrase

                    {
                        unsigned char *reasonPhrase = _workingBuffer->duplicate(_allocator, true);    // trims trailing whitespace

                        if (! reasonPhrase)
                        {
                            return ESF_OUT_OF_MEMORY;
                        }

                        response->setReasonPhrase(reasonPhrase);
                    }

                    return ESF_SUCCESS;

                case ESF_INPROGRESS:

                    // LWS encountered - replace with a single space & trim leading white space

                    if (0 < _workingBuffer->getWritePosition())
                    {
                        _workingBuffer->putNext(' ');
                    }

                    break;

                default:

                    return error;
            }

            continue;
        }

        if (AWSHttpUtil::IsText(octet))
        {
            _workingBuffer->putNext(octet);
            continue;
        }

        return AWS_HTTP_BAD_REASON_PHRASE;
    }
}

bool AWSHttpResponseParser::isBodyNotAllowed(AWSHttpMessage *message)
{
    // For response messages, whether or not a message-body is included with
    // a message is dependent on both the request method and the response
    // status code (section 6.1.1). All responses to the HEAD request method
    // MUST NOT include a message-body, even though the presence of entity-
    // header fields might lead one to believe they do. All 1xx
    // (informational), 204 (no content), and 304 (not modified) responses
    // MUST NOT include a message-body. All other responses do include a
    // message-body, although it MAY be of zero length

    AWSHttpResponse *response = (AWSHttpResponse *) message;

    switch (response->getStatusCode())
    {
        case 204:
        case 304:

            return true;

        default:

            return 200 <= response->getStatusCode();
    }
}


