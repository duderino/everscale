/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_MESSAGE_PARSER_H
#include <AWSHttpMessageParser.h>
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

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define AWS_PARSING_START_LINE (1 << 0)
#define AWS_PARSING_FIELD_NAME (1 << 1)
#define AWS_PARSING_FIELD_VALUE (1 << 2)
#define AWS_HEADER_PARSE_COMPLETE (1 << 3)
#define AWS_PARSING_UNENCODED_BODY (1 << 4)
#define AWS_PARSING_CHUNKED_BODY (1 << 5)
#define AWS_PARSING_CHUNK_SIZE (1 << 6)
#define AWS_PARSING_CHUNK_EXTENSION (1 << 7)
#define AWS_PARSING_CHUNK_DATA (1 << 8)
#define AWS_PARSING_END_CHUNK (1 << 9)
#define AWS_PARSING_TRAILER (1 << 10)
#define AWS_BODY_PARSE_COMPLETE (1 << 11)
#define AWS_PARSING_MULTIPART_BODY (1 << 12)
#define AWS_PARSING_BODY_UNTIL_CLOSE (1 << 13)

AWSHttpMessageParser::AWSHttpMessageParser(ESFBuffer *workingBuffer, ESFDiscardAllocator *allocator) :
    _workingBuffer(workingBuffer),
    _allocator(allocator),
    _state(0x00),
    _bodyBytesRemaining(0)
{
}

AWSHttpMessageParser::~AWSHttpMessageParser()
{
}

void AWSHttpMessageParser::reset()
{
    _workingBuffer->clear();
    _state = 0x00;
    _bodyBytesRemaining = 0;
}

ESFError AWSHttpMessageParser::parseHeaders(ESFBuffer *inputBuffer, AWSHttpMessage *message)
{
    // Request       = Request-Line              ; Section 5.1
    //                *(( general-header        ; Section 4.5
    //                 | message-header         ; Section 5.3
    //                 | entity-header ) CRLF)  ; Section 7.1
    //                CRLF
    //                [ message-body ]          ; Section 4.3

    if (AWS_HEADER_PARSE_COMPLETE & _state)
    {
        return ESF_INVALID_STATE;
    }

    if (0x00 == _state)
    {
        // In the interest of robustness, servers SHOULD ignore any empty
        // line(s) received where a Request-Line is expected. In other words, if
        // the server is reading the protocol stream at the beginning of a
        // message and receives a CRLF first, it should ignore the CRLF.

        while (true)
        {
            if (false == inputBuffer->isReadable())
            {
                return ESF_AGAIN;
            }

            if (AWSHttpUtil::IsLWS(inputBuffer->peekNext()))
            {
                inputBuffer->skipNext();
                continue;
            }

            break;
        }

        _state = AWS_PARSING_START_LINE;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    ESFError error = ESF_SUCCESS;

    if (AWS_PARSING_START_LINE & _state)
    {
        error = parseStartLine(inputBuffer, message);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _state &= ~AWS_PARSING_START_LINE;
        _state |= AWS_PARSING_FIELD_NAME;

        inputBuffer->readMark();
        _workingBuffer->clear();
    }

    unsigned char octet;

    while ((AWS_PARSING_FIELD_NAME | AWS_PARSING_FIELD_VALUE) & _state)
    {
        if (AWS_PARSING_FIELD_NAME & _state)
        {
            // Do we have a final CRLF?

            // The line terminator for message-header fields is the sequence CRLF.
            // However, we recommend that applications, when parsing such headers,
            // recognize a single LF as a line terminator and ignore the leading CR.

            if (false == inputBuffer->isReadable())
            {
                return ESF_AGAIN;
            }

            octet = inputBuffer->peekNext();

            if ('\r' == octet)
            {
                inputBuffer->skipNext();

                if (false == inputBuffer->isReadable())
                {
                    inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);

                    return ESF_AGAIN;
                }

                if ('\n' != inputBuffer->peekNext())
                {
                    return AWS_HTTP_BAD_REQUEST_FIELD_NAME;
                }

                inputBuffer->skipNext();

                _state &= ~AWS_PARSING_FIELD_NAME;
                _state |= AWS_HEADER_PARSE_COMPLETE;

                inputBuffer->readMark();
                _workingBuffer->clear();

                return postParse(message);
            }
            else if ('\n' == octet)
            {
                inputBuffer->skipNext();

                _state &= ~AWS_PARSING_FIELD_NAME;
                _state |= AWS_HEADER_PARSE_COMPLETE;

                inputBuffer->readMark();
                _workingBuffer->clear();

                return postParse(message);
            }

            ESF_ASSERT(' ' != octet);
            ESF_ASSERT('\t' != octet);

            error = parseFieldName(inputBuffer, message);

            if (ESF_SUCCESS != error)
            {
                return error;
            }

            _state &= ~AWS_PARSING_FIELD_NAME;
            _state |= AWS_PARSING_FIELD_VALUE;

            inputBuffer->readMark();
            _workingBuffer->clear();
        }

        if (AWS_PARSING_FIELD_VALUE & _state)
        {
            error = parseFieldValue(inputBuffer, message);

            if (ESF_SUCCESS != error)
            {
                return error;
            }

            _state &= ~AWS_PARSING_FIELD_VALUE;
            _state |= AWS_PARSING_FIELD_NAME;

            inputBuffer->readMark();
            _workingBuffer->clear();
        }
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpMessageParser::parseFieldName(ESFBuffer *inputBuffer, AWSHttpMessage *message)
{
    // field-name     = token

    ESF_ASSERT(AWS_PARSING_FIELD_NAME & _state);

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

        if (':' == octet)
        {
            unsigned char *fieldName = _workingBuffer->duplicate(_allocator);

            if (! fieldName)
            {
                return ESF_OUT_OF_MEMORY;
            }

            AWSHttpHeader *header = new(_allocator) AWSHttpHeader(fieldName, 0);

            if (! header)
            {
                _allocator->deallocate(fieldName);   // no-op

                return ESF_OUT_OF_MEMORY;
            }

            // The next production - parseFieldValue - may fill in in the value

            message->getHeaders()->addLast(header);

            return ESF_SUCCESS;
        }

        if (AWSHttpUtil::IsToken(octet))
        {
            _workingBuffer->putNext(octet);
            continue;
        }

        return AWS_HTTP_BAD_REQUEST_FIELD_NAME;
    }
}

ESFError AWSHttpMessageParser::parseFieldValue(ESFBuffer *inputBuffer, AWSHttpMessage *message)
{
    // field-value    = *( field-content | LWS )
    // field-content  = <the OCTETs making up the field-value
    //                 and consisting of either *TEXT or combinations
    //                 of token, separators, and quoted-string>

    // The field value MAY be preceded by any amount
    // of LWS, though a single SP is preferred. Header fields can be
    // extended over multiple lines by preceding each extra line with at
    // least one SP or HT.

    ESF_ASSERT(AWS_PARSING_FIELD_VALUE & _state);

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
            // The field-content does not include any leading or trailing LWS:
            // linear white space occurring before the first non-whitespace
            // character of the field-value or after the last non-whitespace
            // character of the field-value. Such leading or trailing LWS MAY be
            // removed without changing the semantics of the field value. Any LWS
            // that occurs between field-content MAY be replaced with a single SP
            // before interpreting the field value or forwarding the message
            // downstream.

            inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);

            error = AWSHttpUtil::SkipLWS(inputBuffer);

            switch (error)
            {
                case ESF_SUCCESS:

                    // newline encountered - save field value

                    {
                        unsigned char *fieldValue = _workingBuffer->duplicate(_allocator, true);    // trims trailing whitespace

                        if (! fieldValue)
                        {
                            return ESF_OUT_OF_MEMORY;
                        }

                        AWSHttpHeader *header = (AWSHttpHeader *) message->getHeaders()->getLast();

                        ESF_ASSERT(header);

                        if (! header )
                        {
                            return ESF_INVALID_STATE;
                        }

                        header->setFieldValue(fieldValue);
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

        if (AWSHttpUtil::IsToken(octet))
        {
            _workingBuffer->putNext(octet);
            continue;
        }

        if (AWSHttpUtil::IsSeparator(octet))
        {
            _workingBuffer->putNext(octet);
            continue;
        }

        if (AWSHttpUtil::IsText(octet))
        {
            _workingBuffer->putNext(octet);
            continue;
        }

        return AWS_HTTP_BAD_REQUEST_FIELD_VALUE;
    }
}

ESFError AWSHttpMessageParser::parseBody(ESFBuffer *inputBuffer, int *startingPosition, int *chunkSize)
{
    if (! inputBuffer || ! startingPosition || ! chunkSize)
    {
        return ESF_NULL_POINTER;
    }

    ESF_ASSERT(AWS_HEADER_PARSE_COMPLETE & _state);

    if ((AWS_PARSING_UNENCODED_BODY | AWS_PARSING_BODY_UNTIL_CLOSE) & _state)
    {
        return parseUnencodedBody(inputBuffer, startingPosition, chunkSize);
    }

    if (AWS_PARSING_CHUNKED_BODY & _state)
    {
        return parseChunkedBody(inputBuffer, startingPosition, chunkSize);
    }

    if (AWS_PARSING_MULTIPART_BODY & _state)
    {
        return parseMultipartBody(inputBuffer, startingPosition, chunkSize);
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpMessageParser::parseUnencodedBody(ESFBuffer *inputBuffer, int *startingPosition, int *chunkSize)
{
    // chunk-data     = chunk-size(OCTET)

    if (AWS_BODY_PARSE_COMPLETE & _state)
    {
        _state &= ~(AWS_PARSING_UNENCODED_BODY | AWS_PARSING_BODY_UNTIL_CLOSE);

        *chunkSize = 0;

        return ESF_SUCCESS;
    }

    ESF_ASSERT((AWS_PARSING_UNENCODED_BODY | AWS_PARSING_BODY_UNTIL_CLOSE) & _state);

    if (false == inputBuffer->isReadable())
    {
        return ESF_AGAIN;
    }

    int bytesRemaining = MIN(_bodyBytesRemaining, inputBuffer->getReadable());

    *startingPosition = inputBuffer->getReadPosition();
    *chunkSize = bytesRemaining;

    inputBuffer->skip(bytesRemaining);
    _bodyBytesRemaining -= bytesRemaining;

    if (0 == _bodyBytesRemaining)
    {
        _state |= AWS_BODY_PARSE_COMPLETE;

        inputBuffer->readMark();
    }

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageParser::parseChunkedBody(ESFBuffer *inputBuffer, int *startingPosition, int *chunkSize)
{
    // Chunked-Body   = *chunk
    //                  last-chunk
    //                  trailer
    //                  CRLF

    // chunk          = chunk-size [ chunk-extension ] CRLF
    //                  chunk-data CRLF

    // last-chunk     = 1*("0") [ chunk-extension ] CRLF

    ESF_ASSERT(AWS_PARSING_CHUNKED_BODY & _state);

    ESFError error = ESF_SUCCESS;

    if (AWS_PARSING_END_CHUNK & _state)
    {
        error = parseEndChunk(inputBuffer);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _state &= ~AWS_PARSING_END_CHUNK;
        _state |= AWS_PARSING_CHUNK_SIZE;

        inputBuffer->readMark();
    }

    if (AWS_PARSING_CHUNK_SIZE & _state)
    {
        error = parseChunkSize(inputBuffer);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _state &= ~AWS_PARSING_CHUNK_SIZE;
        _state |= AWS_PARSING_CHUNK_EXTENSION;

        inputBuffer->readMark();
    }

    if (AWS_PARSING_CHUNK_EXTENSION & _state)
    {
        error = parseChunkExtension(inputBuffer);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        _state &= ~AWS_PARSING_CHUNK_EXTENSION;
        inputBuffer->readMark();

        if (0 == _bodyBytesRemaining)
        {
            _state |= AWS_PARSING_TRAILER;

            *chunkSize = 0;

            return ESF_SUCCESS;
        }

        _state |= AWS_PARSING_CHUNK_DATA;
    }

    if (AWS_PARSING_CHUNK_DATA & _state)
    {
        error = parseChunkData(inputBuffer, startingPosition, chunkSize);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        if (0 == _bodyBytesRemaining)
        {
            _state &= ~AWS_PARSING_CHUNK_DATA;
            _state |= AWS_PARSING_END_CHUNK;

            inputBuffer->readMark();
        }

        return ESF_SUCCESS;
    }

    return ESF_INVALID_STATE;
}

ESFError AWSHttpMessageParser::skipTrailer(ESFBuffer *inputBuffer)
{
    // For chunked transfer encoding only, everything else is a no-op
    // Chunked-Body   = ...
    //                  trailer
    //                  CRLF
    // trailer        = *(entity-header CRLF)

    if (!(AWS_PARSING_TRAILER & _state))
    {
        return ESF_SUCCESS;
    }

    if (! inputBuffer)
    {
        return ESF_NULL_POINTER;
    }

    int bytesSkipped = 0;
    ESFError error;

    while (true)
    {
        error = AWSHttpUtil::SkipLine(inputBuffer, &bytesSkipped);

        if (ESF_SUCCESS != error)
        {
            return error;
        }

        if (0 == bytesSkipped)
        {
            _state &= ~AWS_PARSING_TRAILER;
            _state |= AWS_BODY_PARSE_COMPLETE;

            inputBuffer->readMark();

            return ESF_SUCCESS;
        }
    }
}

ESFError AWSHttpMessageParser::parseChunkSize(ESFBuffer *inputBuffer)
{
    // chunk-size     = 1*HEX

    ESF_ASSERT(AWS_PARSING_CHUNK_SIZE & _state);

    if (0 == _bodyBytesRemaining)
    {
        AWSHttpUtil::SkipSpaces(inputBuffer);
    }

    unsigned char octet;

    while (true)
    {
        if (false == inputBuffer->isReadable())
        {
            return ESF_AGAIN;
        }

        octet = inputBuffer->getNext();

        if (false == AWSHttpUtil::IsHex(octet))
        {
            inputBuffer->setReadPosition(inputBuffer->getReadPosition() - 1);

            return ESF_SUCCESS;
        }

        if (AWSHttpUtil::IsUpAlpha(octet))
        {
            _bodyBytesRemaining = (_bodyBytesRemaining * 16) + (octet - 'A' + 10);
        }
        else if (AWSHttpUtil::IsLowAlpha(octet))
        {
            _bodyBytesRemaining = (_bodyBytesRemaining * 16) + (octet - 'a' + 10);
        }
        else
        {
            _bodyBytesRemaining = (_bodyBytesRemaining * 16) + (octet - '0');
        }
    }
}

ESFError AWSHttpMessageParser::parseChunkExtension(ESFBuffer *inputBuffer)
{
    // chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
    // chunk-ext-name = token
    // chunk-ext-val  = token | quoted-string

    ESF_ASSERT(AWS_PARSING_CHUNK_EXTENSION & _state);

    int bytesSkipped = 0;

    return AWSHttpUtil::SkipLine(inputBuffer, &bytesSkipped);
}

ESFError AWSHttpMessageParser::parseChunkData(ESFBuffer *inputBuffer, int *startingPosition, int *chunkSize)
{
    // chunk-data     = chunk-size(OCTET)

    ESF_ASSERT(AWS_PARSING_CHUNK_DATA & _state);
    ESF_ASSERT(0 < _bodyBytesRemaining);

    if (false == inputBuffer->isReadable())
    {
        return ESF_AGAIN;
    }

    int bytesRemaining = MIN(_bodyBytesRemaining, inputBuffer->getReadable());

    *startingPosition = inputBuffer->getReadPosition();
    *chunkSize = bytesRemaining;

    inputBuffer->skip(bytesRemaining);
    _bodyBytesRemaining -= bytesRemaining;

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageParser::parseEndChunk(ESFBuffer *inputBuffer)
{
    // chunk          = ... CRLF

    ESF_ASSERT(AWS_PARSING_END_CHUNK & _state);

    int bytesSkipped = 0;

    return AWSHttpUtil::SkipLine(inputBuffer, &bytesSkipped);
}

ESFError AWSHttpMessageParser::parseMultipartBody(ESFBuffer *inputBuffer, int *startingPosition, int *chunkSize)
{
    //
    // The multipart/byteranges media type includes two or more parts, each
    // with its own Content-Type and Content-Range fields. The required
    // boundary parameter specifies the boundary string used to separate
    // each body-part.
    //
    //       Media Type name:         multipart
    //       Media subtype name:      byteranges
    //       Required parameters:     boundary
    //       Optional parameters:     none
    //       Encoding considerations: only "7bit", "8bit", or "binary" are
    //                                permitted
    //       Security considerations: none
    //
    //
    //   For example:
    //
    //   HTTP/1.1 206 Partial Content
    //   Date: Wed, 15 Nov 1995 06:25:24 GMT
    //   Last-Modified: Wed, 15 Nov 1995 04:58:08 GMT
    //   Content-type: multipart/byteranges; boundary=THIS_STRING_SEPARATES
    //
    //   --THIS_STRING_SEPARATES
    //   Content-type: application/pdf
    //   Content-range: bytes 500-999/8000
    //
    //   ...the first range...
    //   --THIS_STRING_SEPARATES
    //   Content-type: application/pdf
    //   Content-range: bytes 7000-7999/8000
    //
    //   ...the second range
    //   --THIS_STRING_SEPARATES--

    // TODO extract boundary parameter from Content-Type header.
    // Look for '--' in body
    // If found, try to match boundary parameter
    // If match, skip
    // Look for '--', if found end body

    return ESF_OPERATION_NOT_SUPPORTED;
}

ESFError AWSHttpMessageParser::postParse(AWSHttpMessage *message)
{
    ESF_ASSERT(AWS_HEADER_PARSE_COMPLETE & _state);

    ESFUInt64 contentLength = 0;
    bool foundContentLength = false;
    bool foundMultipartByteRange = false;
    bool foundTransferEncodedChunked = false;
    ESFError error = ESF_SUCCESS;

    message->setReuseConnection(1.1 > message->getHttpVersion() ? false : true);
    message->setSend100Continue(false);

    for (AWSHttpHeader *header = (AWSHttpHeader *) message->getHeaders()->getFirst();
         header;
         header = (AWSHttpHeader *) header->getNext())
    {
        if (0 == strcasecmp((const char *) header->getFieldName(), "Expect"))
        {
            if (0 == header->getFieldValue())
            {
                continue;
            }

            if (0 == strncmp((const char *) header->getFieldValue(),
                             "100-continue",
                             sizeof("100-continue") - 1))
            {
                message->setSend100Continue(true);
            }

            continue;
        }

        if (1.1 <= message->getHttpVersion() &&
            0 == strcasecmp((const char *) header->getFieldName(), "Connection"))
        {
            if (0 == header->getFieldValue())
            {
                continue;
            }

            if (0 == strncmp((const char *) header->getFieldValue(),
                             "close",
                             sizeof("close") - 1))
            {
                message->setReuseConnection(false);
            }

            continue;
        }

        if (0 == strcasecmp((const char *) header->getFieldName(), "Content-Length"))
        {
            // 3.If a Content-Length header field (section 14.13) is present, its
            // decimal value in OCTETs represents both the entity-length and the
            // transfer-length. The Content-Length header field MUST NOT be sent
            // if these two lengths are different (i.e., if a Transfer-Encoding
            // header field is present). If a message is received with both a
            // Transfer-Encoding header field and a Content-Length header field,
            // the latter MUST be ignored.

            if (0 == header->getFieldValue())
            {
                continue;
            }

            error = AWSHttpUtil::ParseInteger(header->getFieldValue(), &contentLength);

            if (AWS_HTTP_BAD_INTEGER == error)
            {
                return AWS_HTTP_BAD_CONTENT_LENGTH;
            }

            if (ESF_SUCCESS != error)
            {
                return error;
            }

            foundContentLength = true;

            continue;
        }

        if (0 == strcasecmp((const char *) header->getFieldName(), "Transfer-Encoding"))
        {
            // 2.If a Transfer-Encoding header field (section 14.41) is present and
            // has any value other than "identity", then the transfer-length is
            // defined by use of the "chunked" transfer-coding (section 3.6),
            // unless the message is terminated by closing the connection.

            // Messages MUST NOT include both a Content-Length header field and a
            // non-identity transfer-coding. If the message does include a non-
            // identity transfer-coding, the Content-Length MUST be ignored.

            if (0 == header->getFieldValue())
            {
                continue;
            }

            if (0 != strncmp((const char *) header->getFieldValue(),
                             "identity",
                             sizeof("identity") - 1))
            {
                foundTransferEncodedChunked = true;
            }

            continue;
        }

        if (0 == strcasecmp((const char *) header->getFieldName(), "Content-Type"))
        {
            // 4.If the message uses the media type "multipart/byteranges", and the
            // transfer-length is not otherwise specified, then this self-
            // delimiting media type defines the transfer-length. This media type
            // MUST NOT be used unless the sender knows that the recipient can parse
            // it; the presence in a message of a Range header with multiple byte-
            // range specifiers from a 1.1 client implies that the client can parse
            // multipart/byteranges responses.

            if (0 == header->getFieldValue())
            {
                continue;
            }

            if (0 == strncmp((const char *) header->getFieldValue(),
                             "multipart/byteranges",
                             sizeof("multipart/byteranges") - 1))
            {
                foundMultipartByteRange = true;
            }

            continue;
        }
    }

    // 5.By the server closing the connection. (Closing the connection
    // cannot be used to indicate the end of a message body, since that
    // would leave no possibility for the server to send back a response.)

    if (foundTransferEncodedChunked)
    {
        _bodyBytesRemaining = 0;
        _state |= AWS_PARSING_CHUNKED_BODY | AWS_PARSING_CHUNK_SIZE;

        message->setHasBody(true);
    }
    else if (foundContentLength)
    {
        _bodyBytesRemaining = contentLength;
        _state |= AWS_PARSING_UNENCODED_BODY;

        message->setHasBody(0 < contentLength);
    }
    else if (foundMultipartByteRange)
    {
        _bodyBytesRemaining = 0;
        _state |= AWS_PARSING_MULTIPART_BODY;

        message->setHasBody(false);

        // TODO - support multipart byte range  bodies
        return AWS_HTTP_MULTIPART_NOT_SUPPORTED;
    }
    else if (isBodyNotAllowed(message))
    {
        // For response messages, whether or not a message-body is included with
        // a message is dependent on both the request method and the response
        // status code (section 6.1.1). All responses to the HEAD request method
        // MUST NOT include a message-body, even though the presence of entity-
        // header fields might lead one to believe they do. All 1xx
        // (informational), 204 (no content), and 304 (not modified) responses
        // MUST NOT include a message-body. All other responses do include a
        // message-body, although it MAY be of zero length

        _bodyBytesRemaining = 0;
        _state |= AWS_PARSING_UNENCODED_BODY;

        message->setHasBody(false);
    }
    else
    {
        // Just read as much as we can until the socket is closed.
        _bodyBytesRemaining = ESF_UINT64_MAX;
        _state |= AWS_PARSING_BODY_UNTIL_CLOSE;

        message->setHasBody(true);
        message->setReuseConnection(false);
    }

    return ESF_SUCCESS;
}

ESFError AWSHttpMessageParser::parseVersion(ESFBuffer *inputBuffer, AWSHttpMessage *message, bool clientMode)
{
    // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

    // Clients SHOULD be tolerant in parsing the Status-Line and servers
    // tolerant when parsing the Request-Line. In particular, they SHOULD
    // accept any amount of SP or HT characters between fields, even though
    // only a single SP is required.

    AWSHttpUtil::SkipSpaces(inputBuffer);

    if (clientMode)
    {
        if (sizeof("HTTP/1.1") > inputBuffer->getReadable())
        {
            return ESF_AGAIN;
        }
    }
    else
    {
        if (sizeof("HTTP/1.1") + 1 > inputBuffer->getReadable())
        {
            return ESF_AGAIN;
        }
    }

    int version;

    if (AWSHttpUtil::IsMatch(inputBuffer, (const unsigned char *) "HTTP/1.1"))
    {
        inputBuffer->skip(sizeof("HTTP/1.1") - 1);

        version = 110;
    }
    else if (AWSHttpUtil::IsMatch(inputBuffer, (const unsigned char *) "HTTP/1.0"))
    {
        inputBuffer->skip(sizeof("HTTP/1.0") - 1);

        version = 100;
    }
    else
    {
        return AWS_HTTP_BAD_REQUEST_VERSION;
    }

    if (clientMode)
    {
        if (false == AWSHttpUtil::IsSpace(inputBuffer->getNext()))
        {
            return AWS_HTTP_BAD_REQUEST_VERSION;
        }
    }
    else
    {
        switch (inputBuffer->getNext())
        {
            case '\n':

                break;

            case '\r':

                if ('\n' != inputBuffer->getNext())
                {
                    return AWS_HTTP_BAD_REQUEST_VERSION;
                }

                break;

            default:

                return AWS_HTTP_BAD_REQUEST_VERSION;
        }
    }

    message->setHttpVersion(version);

    return ESF_SUCCESS;
}



