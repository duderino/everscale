/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_MESSAGE_FORMATTER_H
#include <AWSHttpMessageFormatter.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef AWS_HTTP_ERROR_H
#include <AWSHttpError.h>
#endif

#ifndef AWS_HTTP_UTIL_H
#include <AWSHttpUtil.h>
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define AWS_FORMATTING_START_LINE (1 << 0)
#define AWS_FORMATTING_FIELD_NAME (1 << 1)
#define AWS_FORMATTING_FIELD_VALUE (1 << 2)
#define AWS_HEADER_FORMAT_COMPLETE (1 << 3)
#define AWS_FORMATTING_UNENCODED_BODY (1 << 4)
#define AWS_FORMATTING_CHUNKED_BODY (1 << 5)
#define AWS_BODY_FORMAT_COMPLETE (1 << 6)
#define AWS_FORMATTING_HEADER (AWS_FORMATTING_FIELD_NAME | AWS_FORMATTING_FIELD_VALUE)
#define AWS_FOUND_CONTENT_LENGTH_HEADER (1 << 10)
#define AWS_FOUND_TRANSFER_ENCODING_CHUNKED_HEADER (1 << 11)

AWSHttpMessageFormatter::AWSHttpMessageFormatter() :
    _state(0x00),
    _currentHeader(0)
{
}

AWSHttpMessageFormatter::~AWSHttpMessageFormatter()
{
}

void AWSHttpMessageFormatter::reset()
{
    _state = 0x00;
    _currentHeader = 0;
}

ESFError AWSHttpMessageFormatter::formatHeaders(ESFBuffer *outputBuffer, const AWSHttpMessage *message)
{
     // generic-message = start-line
     //                   *(message-header CRLF)
     //                   CRLF
     //                   [ message-body ]

    if (AWS_HEADER_FORMAT_COMPLETE & _state)
    {
        return ESF_INVALID_STATE;
    }

    if (0x00 == _state)
    {
        if (false == outputBuffer->isWritable())
        {
            return ESF_AGAIN;
        }

        AWSHttpUtil::Start(&_state, outputBuffer, AWS_FORMATTING_START_LINE);
    }

    ESFError error = ESF_SUCCESS;

    if (AWS_FORMATTING_START_LINE & _state)
    {
        error = formatStartLine(outputBuffer, message);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _currentHeader = (AWSHttpHeader *) message->getHeaders()->getFirst();

        AWSHttpUtil::Transition(&_state, outputBuffer, AWS_FORMATTING_START_LINE, AWS_FORMATTING_FIELD_NAME);
    }

    while (AWS_FORMATTING_HEADER & _state)
    {
        if (0 == _currentHeader)
        {
            ESF_ASSERT(AWS_FORMATTING_FIELD_NAME & _state);

            if (2 > outputBuffer->getWritable())
            {
                outputBuffer->writeReset();
                return ESF_AGAIN;
            }

            outputBuffer->putNext('\r');
            outputBuffer->putNext('\n');

            // Messages MUST NOT include both a Content-Length header field and a
            // non-identity transfer-coding. If the message does include a non-
            // identity transfer-coding, the Content-Length MUST be ignored.

            if (AWS_FOUND_TRANSFER_ENCODING_CHUNKED_HEADER & _state)
            {
                return AWSHttpUtil::Transition(&_state, outputBuffer, AWS_FORMATTING_FIELD_NAME, AWS_HEADER_FORMAT_COMPLETE | AWS_FORMATTING_CHUNKED_BODY);
            }
            else
            {
                return AWSHttpUtil::Transition(&_state, outputBuffer, AWS_FORMATTING_FIELD_NAME, AWS_HEADER_FORMAT_COMPLETE | AWS_FORMATTING_UNENCODED_BODY);
            }
        }

        if (AWS_FORMATTING_FIELD_NAME & _state)
        {
            ESF_ASSERT(_currentHeader->getFieldName());

            if (0 == _currentHeader->getFieldName())
            {
                return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REQUEST_FIELD_NAME);
            }

            error = formatFieldName(outputBuffer, _currentHeader->getFieldName());

            if (ESF_SUCCESS != error)
            {
                return error;
            }

            if (0 == strcasecmp((const char *) _currentHeader->getFieldName(), "Content-Length"))
            {
                _state |= AWS_FOUND_CONTENT_LENGTH_HEADER;
            }

            if (0 == strcasecmp((const char *) _currentHeader->getFieldName(), "Transfer-Encoding"))
            {
                // If a Transfer-Encoding header field (section 14.41) is present and
                // has any value other than "identity", then the transfer-length is
                // defined by use of the "chunked" transfer-coding (section 3.6),
                // unless the message is terminated by closing the connection.

                if (0 == _currentHeader->getFieldValue() ||
                    0 != strncmp((const char *) _currentHeader->getFieldValue(), "identity", sizeof("identity") - 1))
                {
                    _state |= AWS_FOUND_TRANSFER_ENCODING_CHUNKED_HEADER;
                }
            }

            AWSHttpUtil::Transition(&_state, outputBuffer, AWS_FORMATTING_FIELD_NAME, AWS_FORMATTING_FIELD_VALUE);
        }

        if (AWS_FORMATTING_FIELD_VALUE & _state)
        {
            error = formatFieldValue(outputBuffer, _currentHeader->getFieldValue());

            if (ESF_SUCCESS != error)
            {
                return error;
            }

            AWSHttpUtil::Transition(&_state, outputBuffer, AWS_FORMATTING_FIELD_VALUE, AWS_FORMATTING_FIELD_NAME);

            _currentHeader = (const AWSHttpHeader *) _currentHeader->getNext();
        }
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpMessageFormatter::formatVersion(ESFBuffer *outputBuffer, const AWSHttpMessage *message, bool clientMode)
{
    // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

    if (clientMode)
    {
        if (false == outputBuffer->isWritable())
        {
            return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
        }

        outputBuffer->putNext(' ');
    }

    const unsigned char *version = 0;

    if (110 == message->getHttpVersion())
    {
        version = (const unsigned char *) "HTTP/1.1";
    }
    else if (100 == message->getHttpVersion())
    {
        version = (const unsigned char *) "HTTP/1.0";
    }
    else
    {
        return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REQUEST_VERSION);
    }

    for (const unsigned char *p = version; *p; ++p)
    {
        if (false == outputBuffer->isWritable())
        {
            return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
        }

        outputBuffer->putNext(*p);
    }

    if (clientMode)
    {
        if (2 > outputBuffer->getWritable())
        {
            return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
        }

        outputBuffer->putNext('\r');
        outputBuffer->putNext('\n');
    }
    else
    {
        if (false == outputBuffer->isWritable())
        {
            return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
        }

        outputBuffer->putNext(' ');
    }

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageFormatter::formatFieldName(ESFBuffer *outputBuffer, const unsigned char *fieldName)
{
    // field-name     = token

    ESF_ASSERT(AWS_FORMATTING_FIELD_NAME & _state);

    for (const unsigned char *p = fieldName; *p; ++p)
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

        return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REQUEST_FIELD_NAME);
    }

    if (2 > outputBuffer->getWritable())
    {
        return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext(':');
    outputBuffer->putNext(' ');

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageFormatter::formatFieldValue(ESFBuffer *outputBuffer, const unsigned char *fieldValue)
{
    // field-value    = *( field-content | LWS )
    // field-content  = <the OCTETs making up the field-value
    //                 and consisting of either *TEXT or combinations
    //                 of token, separators, and quoted-string>

    ESF_ASSERT(AWS_FORMATTING_FIELD_VALUE & _state);

    bool lastIsSpace = true;

    for (const unsigned char *p = fieldValue; *p; ++p)
    {
        if (false == outputBuffer->isWritable())
        {
            return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
        }

        if (AWSHttpUtil::IsLWS(*p))
        {
            // Replace any line continuations, etc. with a single ' '

            if (false == lastIsSpace)
            {
                outputBuffer->putNext(' ');
                lastIsSpace = true;
            }

            continue;
        }

        lastIsSpace = false;

        if (AWSHttpUtil::IsToken(*p))
        {
            outputBuffer->putNext(*p);
            continue;
        }

        if (AWSHttpUtil::IsSeparator(*p))
        {
            outputBuffer->putNext(*p);
            continue;
        }

        if (AWSHttpUtil::IsText(*p))
        {
            outputBuffer->putNext(*p);
            continue;
        }

        return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REQUEST_FIELD_VALUE);
    }

    if (2 > outputBuffer->getWritable())
    {
        return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext('\r');
    outputBuffer->putNext('\n');

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageFormatter::beginBlock(ESFBuffer *outputBuffer, int requestedSize, int *availableSize)
{
    if (! outputBuffer || ! availableSize)
    {
        return ESF_NULL_POINTER;
    }

    if (0 == requestedSize)
    {
        return ESF_INVALID_ARGUMENT;
    }

    if (AWS_BODY_FORMAT_COMPLETE & _state)
    {
        return ESF_INVALID_STATE;
    }

    if (AWS_FORMATTING_CHUNKED_BODY & _state)
    {
        return beginChunk(outputBuffer, requestedSize, availableSize);
    }

    if (AWS_FORMATTING_UNENCODED_BODY & _state)
    {
        return beginUnencodedBlock(outputBuffer, requestedSize, availableSize);
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpMessageFormatter::endBlock(ESFBuffer *outputBuffer)
{
    if (! outputBuffer)
    {
        return ESF_NULL_POINTER;
    }

    if (AWS_BODY_FORMAT_COMPLETE & _state)
    {
        return ESF_INVALID_STATE;
    }

    if (AWS_FORMATTING_CHUNKED_BODY & _state)
    {
        return endChunk(outputBuffer);
    }

    if (AWS_FORMATTING_UNENCODED_BODY & _state)
    {
        return ESF_SUCCESS;
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpMessageFormatter::endBody(ESFBuffer *outputBuffer)
{
    if (! outputBuffer)
    {
        return ESF_NULL_POINTER;
    }

    if (AWS_BODY_FORMAT_COMPLETE & _state)
    {
        return ESF_INVALID_STATE;
    }

    if (AWS_FORMATTING_CHUNKED_BODY & _state)
    {
        // Chunked-Body   = ...
        //                  last-chunk
        //                  CRLF
        // last-chunk     = 1*("0") [ chunk-extension ] CRLF

        if (5 > outputBuffer->getWritable())
        {
            return ESF_AGAIN;
        }

        outputBuffer->putNext('0');
        outputBuffer->putNext('\r');
        outputBuffer->putNext('\n');
        outputBuffer->putNext('\r');
        outputBuffer->putNext('\n');

        return AWSHttpUtil::Transition(&_state, outputBuffer, AWS_FORMATTING_CHUNKED_BODY, AWS_BODY_FORMAT_COMPLETE);
    }

    if (AWS_FORMATTING_UNENCODED_BODY & _state)
    {
        return AWSHttpUtil::Transition(&_state, outputBuffer, AWS_FORMATTING_UNENCODED_BODY, AWS_BODY_FORMAT_COMPLETE);
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpMessageFormatter::beginChunk(ESFBuffer *outputBuffer, int requestedSize, int *availableSize)
{
    // chunk          = chunk-size [ chunk-extension ] CRLF
    //                  ...
    // chunk-size     = 1*HEX

    ESF_ASSERT(AWS_FORMATTING_CHUNKED_BODY & _state);
    ESF_ASSERT(0 < requestedSize);
    ESF_ASSERT(availableSize);

    // reserve characters for chunk-size and the CRLF after the chunk data
    // Max supported chunk-size is 0xFFFFFFFF.  So reserve 8 for chunk-size
    // + 2 for the CRLF in the chunk-size production + 2 for the CRLF after
    // the chunk data.

    if (0 >= ((int) outputBuffer->getWritable()) - 12)
    {
        return ESF_AGAIN;
    }

    *availableSize = MIN(requestedSize, ((int) outputBuffer->getWritable()) - 12);

    ESF_ASSERT(0 < *availableSize);

    ESFError error = AWSHttpUtil::FormatInteger(outputBuffer, *availableSize, 16);

    if (ESF_SUCCESS != error)
    {
        return AWSHttpUtil::Rollback(outputBuffer, error);
    }

    if (2 > outputBuffer->getWritable())
    {
        return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    outputBuffer->putNext('\r');
    outputBuffer->putNext('\n');

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageFormatter::endChunk(ESFBuffer *outputBuffer)
{
    // chunk          = ...
    //                  chunk-data CRLF
    // chunk-data     = chunk-size(OCTET)

    ESF_ASSERT(AWS_FORMATTING_CHUNKED_BODY & _state);

    if (2 > outputBuffer->getWritable())
    {
        return ESF_AGAIN;
    }

    outputBuffer->putNext('\r');
    outputBuffer->putNext('\n');

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageFormatter::beginUnencodedBlock(ESFBuffer *outputBuffer, int requestedSize, int *availableSize)
{
    ESF_ASSERT(AWS_FORMATTING_UNENCODED_BODY & _state);
    ESF_ASSERT(0 < requestedSize);
    ESF_ASSERT(availableSize);

    if (0 >= outputBuffer->getWritable())
    {
        return ESF_AGAIN;
    }

    *availableSize = MIN((unsigned int) requestedSize, outputBuffer->getWritable());

    return ESF_SUCCESS;
}




