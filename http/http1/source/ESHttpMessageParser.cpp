#ifndef ES_HTTP_MESSAGE_PARSER_H
#include <ESHttpMessageParser.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#define ES_PARSING_START_LINE (1 << 0)
#define ES_PARSING_FIELD_NAME (1 << 1)
#define ES_PARSING_FIELD_VALUE (1 << 2)
#define ES_HEADER_PARSE_COMPLETE (1 << 3)
#define ES_PARSING_UNENCODED_BODY (1 << 4)
#define ES_PARSING_CHUNKED_BODY (1 << 5)
#define ES_PARSING_CHUNK_SIZE (1 << 6)
#define ES_PARSING_CHUNK_EXTENSION (1 << 7)
#define ES_PARSING_CHUNK_DATA (1 << 8)
#define ES_PARSING_END_CHUNK (1 << 9)
#define ES_PARSING_TRAILER (1 << 10)
#define ES_BODY_PARSE_COMPLETE (1 << 11)
#define ES_PARSING_MULTIPART_BODY (1 << 12)
#define ES_PARSING_BODY_UNTIL_CLOSE (1 << 13)

HttpMessageParser::HttpMessageParser(ESB::Buffer *workingBuffer,
                                     ESB::DiscardAllocator &allocator)
    : _workingBuffer(workingBuffer),
      _allocator(allocator),
      _state(0x00),
      _bodyBytesRemaining(0) {}

HttpMessageParser::~HttpMessageParser() {}

void HttpMessageParser::reset() {
  _workingBuffer->clear();
  _state = 0x00;
  _bodyBytesRemaining = 0;
}

ESB::Error HttpMessageParser::parseHeaders(ESB::Buffer *inputBuffer,
                                           HttpMessage &message) {
  // Request       = Request-Line              ; Section 5.1
  //                *(( general-header        ; Section 4.5
  //                 | message-header         ; Section 5.3
  //                 | entity-header ) CRLF)  ; Section 7.1
  //                CRLF
  //                [ message-body ]          ; Section 4.3

  if (ES_HEADER_PARSE_COMPLETE & _state) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _state) {
    // In the interest of robustness, servers SHOULD ignore any empty
    // line(s) received where a Request-Line is expected. In other words, if
    // the server is reading the protocol stream at the beginning of a
    // message and receives a CRLF first, it should ignore the CRLF.

    while (true) {
      if (!inputBuffer->isReadable()) {
        return ESB_AGAIN;
      }

      if (HttpUtil::IsLWS(inputBuffer->peekNext())) {
        inputBuffer->skipNext();
        continue;
      }

      break;
    }

    _state = ES_PARSING_START_LINE;

    inputBuffer->readMark();
    _workingBuffer->clear();
  }

  ESB::Error error = ESB_SUCCESS;

  if (ES_PARSING_START_LINE & _state) {
    error = parseStartLine(inputBuffer, message);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _state &= ~ES_PARSING_START_LINE;
    _state |= ES_PARSING_FIELD_NAME;

    inputBuffer->readMark();
    _workingBuffer->clear();
  }

  unsigned char octet;

  while ((ES_PARSING_FIELD_NAME | ES_PARSING_FIELD_VALUE) & _state) {
    if (ES_PARSING_FIELD_NAME & _state) {
      // Do we have a final CRLF?

      // The line terminator for message-header fields is the sequence CRLF.
      // However, we recommend that applications, when parsing such headers,
      // recognize a single LF as a line terminator and ignore the leading CR.

      if (!inputBuffer->isReadable()) {
        return ESB_AGAIN;
      }

      octet = inputBuffer->peekNext();

      if ('\r' == octet) {
        inputBuffer->skipNext();

        if (!inputBuffer->isReadable()) {
          inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);

          return ESB_AGAIN;
        }

        if ('\n' != inputBuffer->peekNext()) {
          return ES_HTTP_BAD_REQUEST_FIELD_NAME;
        }

        inputBuffer->skipNext();

        _state &= ~ES_PARSING_FIELD_NAME;
        _state |= ES_HEADER_PARSE_COMPLETE;

        inputBuffer->readMark();
        _workingBuffer->clear();

        return postParse(message);
      } else if ('\n' == octet) {
        inputBuffer->skipNext();

        _state &= ~ES_PARSING_FIELD_NAME;
        _state |= ES_HEADER_PARSE_COMPLETE;

        inputBuffer->readMark();
        _workingBuffer->clear();

        return postParse(message);
      }

      assert(' ' != octet);
      assert('\t' != octet);

      error = parseFieldName(inputBuffer, message);

      if (ESB_SUCCESS != error) {
        return error;
      }

      _state &= ~ES_PARSING_FIELD_NAME;
      _state |= ES_PARSING_FIELD_VALUE;

      inputBuffer->readMark();
      _workingBuffer->clear();
    }

    if (ES_PARSING_FIELD_VALUE & _state) {
      error = parseFieldValue(inputBuffer, message);

      if (ESB_SUCCESS != error) {
        return error;
      }

      _state &= ~ES_PARSING_FIELD_VALUE;
      _state |= ES_PARSING_FIELD_NAME;

      inputBuffer->readMark();
      _workingBuffer->clear();
    }
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpMessageParser::parseFieldName(ESB::Buffer *inputBuffer,
                                             HttpMessage &message) {
  // field-name     = token

  assert(ES_PARSING_FIELD_NAME & _state);

  unsigned char octet;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_workingBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (':' == octet) {
      unsigned char *fieldName = _workingBuffer->duplicate(_allocator);

      if (!fieldName) {
        return ESB_OUT_OF_MEMORY;
      }

      HttpHeader *header = new (_allocator) HttpHeader(fieldName);

      if (!header) {
        _allocator.deallocate(fieldName);  // no-op

        return ESB_OUT_OF_MEMORY;
      }

      // The next production - parseFieldValue - may fill in in the value

      message.headers().addLast(header);

      return ESB_SUCCESS;
    }

    if (HttpUtil::IsToken(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    return ES_HTTP_BAD_REQUEST_FIELD_NAME;
  }
}

ESB::Error HttpMessageParser::parseFieldValue(ESB::Buffer *inputBuffer,
                                              HttpMessage &message) {
  // field-value    = *( field-content | LWS )
  // field-content  = <the OCTETs making up the field-value
  //                 and consisting of either *TEXT or combinations
  //                 of token, separators, and quoted-string>

  // The field value MAY be preceded by any amount
  // of LWS, though a single SP is preferred. Header fields can be
  // extended over multiple lines by preceding each extra line with at
  // least one SP or HT.

  assert(ES_PARSING_FIELD_VALUE & _state);

  ESB::Error error;
  unsigned char octet;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_workingBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (HttpUtil::IsLWS(octet)) {
      // The field-content does not include any leading or trailing LWS:
      // linear white space occurring before the first non-whitespace
      // character of the field-value or after the last non-whitespace
      // character of the field-value. Such leading or trailing LWS MAY be
      // removed without changing the semantics of the field value. Any LWS
      // that occurs between field-content MAY be replaced with a single SP
      // before interpreting the field value or forwarding the message
      // downstream.

      inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);

      error = HttpUtil::SkipLWS(inputBuffer);

      switch (error) {
        case ESB_SUCCESS:

          // newline encountered - save field value

          {
            unsigned char *fieldValue =
                _workingBuffer->duplicate(_allocator, true);

            if (!fieldValue) {
              return ESB_OUT_OF_MEMORY;
            }

            HttpHeader *header = (HttpHeader *)message.headers().last();

            assert(header);

            if (!header) {
              return ESB_INVALID_STATE;
            }

            header->setFieldValue(fieldValue);
          }

          return ESB_SUCCESS;

        case ESB_INPROGRESS:

          // LWS encountered - replace with a single space & trim leading white
          // space

          if (0 < _workingBuffer->writePosition()) {
            _workingBuffer->putNext(' ');
          }

          break;

        default:

          return error;
      }

      continue;
    }

    if (HttpUtil::IsToken(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    if (HttpUtil::IsSeparator(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    if (HttpUtil::IsText(octet)) {
      _workingBuffer->putNext(octet);
      continue;
    }

    return ES_HTTP_BAD_REQUEST_FIELD_VALUE;
  }
}

ESB::Error HttpMessageParser::parseBody(ESB::Buffer *inputBuffer,
                                        ESB::UInt32 *startingPosition,
                                        ESB::UInt32 *chunkSize) {
  if (!inputBuffer || !startingPosition || !chunkSize) {
    return ESB_NULL_POINTER;
  }

  assert(ES_HEADER_PARSE_COMPLETE & _state);

  if ((ES_PARSING_UNENCODED_BODY | ES_PARSING_BODY_UNTIL_CLOSE) & _state) {
    return parseUnencodedBody(inputBuffer, startingPosition, chunkSize);
  }

  if (ES_PARSING_CHUNKED_BODY & _state) {
    return parseChunkedBody(inputBuffer, startingPosition, chunkSize);
  }

  if (ES_PARSING_MULTIPART_BODY & _state) {
    return parseMultipartBody(inputBuffer, startingPosition, chunkSize);
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpMessageParser::consumeBody(ESB::Buffer *inputBuffer,
                                          ESB::UInt32 bytesConsumed) {
  if (!inputBuffer) {
    return ESB_NULL_POINTER;
  }

  assert(ES_HEADER_PARSE_COMPLETE & _state);

  if (ES_PARSING_MULTIPART_BODY & _state) {
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  if ((ES_PARSING_UNENCODED_BODY | ES_PARSING_BODY_UNTIL_CLOSE) & _state) {
    inputBuffer->skip(bytesConsumed);
    _bodyBytesRemaining -= bytesConsumed;

    if (0 == _bodyBytesRemaining) {
      _state |= ES_BODY_PARSE_COMPLETE;
      inputBuffer->readMark();
    }

    return ESB_SUCCESS;
  }

  assert(ES_PARSING_CHUNKED_BODY & _state);

  if (!(ES_PARSING_CHUNK_DATA | _state)) {
    return ESB_INVALID_STATE;
  }

  inputBuffer->skip(bytesConsumed);
  _bodyBytesRemaining -= bytesConsumed;

  if (0 == _bodyBytesRemaining) {
    _state &= ~ES_PARSING_CHUNK_DATA;
    _state |= ES_PARSING_END_CHUNK;
    inputBuffer->readMark();
  }

  return ESB_SUCCESS;
}

ESB::Error HttpMessageParser::parseUnencodedBody(ESB::Buffer *inputBuffer,
                                                 ESB::UInt32 *startingPosition,
                                                 ESB::UInt32 *chunkSize) {
  // chunk-data     = chunk-size(OCTET)

  if (ES_BODY_PARSE_COMPLETE & _state) {
    _state &= ~(ES_PARSING_UNENCODED_BODY | ES_PARSING_BODY_UNTIL_CLOSE);

    *chunkSize = 0;

    return ESB_SUCCESS;
  }

  assert((ES_PARSING_UNENCODED_BODY | ES_PARSING_BODY_UNTIL_CLOSE) & _state);

  if (false == inputBuffer->isReadable()) {
    return ESB_AGAIN;
  }

  ESB::UInt32 bytesRemaining =
      MIN(_bodyBytesRemaining, inputBuffer->readable());

  *startingPosition = inputBuffer->readPosition();
  *chunkSize = bytesRemaining;

  return ESB_SUCCESS;
}

ESB::Error HttpMessageParser::parseChunkedBody(ESB::Buffer *inputBuffer,
                                               ESB::UInt32 *startingPosition,
                                               ESB::UInt32 *chunkSize) {
  // Chunked-Body   = *chunk
  //                  last-chunk
  //                  trailer
  //                  CRLF

  // chunk          = chunk-size [ chunk-extension ] CRLF
  //                  chunk-data CRLF

  // last-chunk     = 1*("0") [ chunk-extension ] CRLF

  assert(ES_PARSING_CHUNKED_BODY & _state);

  ESB::Error error = ESB_SUCCESS;

  if (ES_PARSING_END_CHUNK & _state) {
    error = parseEndChunk(inputBuffer);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _state &= ~ES_PARSING_END_CHUNK;
    _state |= ES_PARSING_CHUNK_SIZE;

    inputBuffer->readMark();
  }

  if (ES_PARSING_CHUNK_SIZE & _state) {
    error = parseChunkSize(inputBuffer);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _state &= ~ES_PARSING_CHUNK_SIZE;
    _state |= ES_PARSING_CHUNK_EXTENSION;

    inputBuffer->readMark();
  }

  if (ES_PARSING_CHUNK_EXTENSION & _state) {
    error = parseChunkExtension(inputBuffer);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _state &= ~ES_PARSING_CHUNK_EXTENSION;
    inputBuffer->readMark();

    if (0 == _bodyBytesRemaining) {
      _state |= ES_PARSING_TRAILER;

      *chunkSize = 0;

      return ESB_SUCCESS;
    }

    _state |= ES_PARSING_CHUNK_DATA;
  }

  if (ES_PARSING_CHUNK_DATA & _state) {
    return parseChunkData(inputBuffer, startingPosition, chunkSize);
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpMessageParser::skipTrailer(ESB::Buffer *inputBuffer) {
  // For chunked transfer encoding only, everything else is a no-op
  // Chunked-Body   = ...
  //                  trailer
  //                  CRLF
  // trailer        = *(entity-header CRLF)

  if (!(ES_PARSING_TRAILER & _state)) {
    return ESB_SUCCESS;
  }

  if (!inputBuffer) {
    return ESB_NULL_POINTER;
  }

  ESB::UInt32 bytesSkipped = 0;
  ESB::Error error;

  while (true) {
    error = HttpUtil::SkipLine(inputBuffer, &bytesSkipped);

    if (ESB_SUCCESS != error) {
      return error;
    }

    if (0 == bytesSkipped) {
      _state &= ~ES_PARSING_TRAILER;
      _state |= ES_BODY_PARSE_COMPLETE;

      inputBuffer->readMark();

      return ESB_SUCCESS;
    }
  }
}

ESB::Error HttpMessageParser::parseChunkSize(ESB::Buffer *inputBuffer) {
  // chunk-size     = 1*HEX

  assert(ES_PARSING_CHUNK_SIZE & _state);

  if (0 == _bodyBytesRemaining) {
    HttpUtil::SkipSpaces(inputBuffer);
  }

  unsigned char octet;

  while (true) {
    if (false == inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    octet = inputBuffer->next();

    if (false == HttpUtil::IsHex(octet)) {
      inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);

      return ESB_SUCCESS;
    }

    if (HttpUtil::IsUpAlpha(octet)) {
      _bodyBytesRemaining = (_bodyBytesRemaining * 16) + (octet - 'A' + 10);
    } else if (HttpUtil::IsLowAlpha(octet)) {
      _bodyBytesRemaining = (_bodyBytesRemaining * 16) + (octet - 'a' + 10);
    } else {
      _bodyBytesRemaining = (_bodyBytesRemaining * 16) + (octet - '0');
    }
  }
}

ESB::Error HttpMessageParser::parseChunkExtension(ESB::Buffer *inputBuffer) {
  // chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
  // chunk-ext-name = token
  // chunk-ext-val  = token | quoted-string

  assert(ES_PARSING_CHUNK_EXTENSION & _state);
  ESB::UInt32 bytesSkipped = 0;
  return HttpUtil::SkipLine(inputBuffer, &bytesSkipped);
}

ESB::Error HttpMessageParser::parseChunkData(ESB::Buffer *inputBuffer,
                                             ESB::UInt32 *startingPosition,
                                             ESB::UInt32 *chunkSize) {
  // chunk-data     = chunk-size(OCTET)

  assert(ES_PARSING_CHUNK_DATA & _state);
  assert(0 < _bodyBytesRemaining);

  if (false == inputBuffer->isReadable()) {
    return ESB_AGAIN;
  }

  *startingPosition = inputBuffer->readPosition();
  *chunkSize = MIN(_bodyBytesRemaining, inputBuffer->readable());

  return ESB_SUCCESS;
}

ESB::Error HttpMessageParser::parseEndChunk(ESB::Buffer *inputBuffer) {
  // chunk          = ... CRLF

  assert(ES_PARSING_END_CHUNK & _state);
  ESB::UInt32 bytesSkipped = 0;
  return HttpUtil::SkipLine(inputBuffer, &bytesSkipped);
}

ESB::Error HttpMessageParser::parseMultipartBody(ESB::Buffer *inputBuffer,
                                                 ESB::UInt32 *startingPosition,
                                                 ESB::UInt32 *chunkSize) {
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

  return ESB_OPERATION_NOT_SUPPORTED;
}

ESB::Error HttpMessageParser::postParse(HttpMessage &message) {
  assert(ES_HEADER_PARSE_COMPLETE & _state);

  ESB::UInt64 contentLength = 0;
  bool foundContentLength = false;
  bool foundMultipartByteRange = false;
  bool foundTransferEncodedChunked = false;
  ESB::Error error = ESB_SUCCESS;

  message.setReuseConnection(1.1 <= message.httpVersion());
  message.setSend100Continue(false);

  for (HttpHeader *header = (HttpHeader *)message.headers().first(); header;
       header = (HttpHeader *)header->next()) {
    if (0 == strcasecmp((const char *)header->fieldName(), "Expect")) {
      if (0 == header->fieldValue()) {
        continue;
      }

      if (0 == strncmp((const char *)header->fieldValue(), "100-continue",
                       sizeof("100-continue") - 1)) {
        message.setSend100Continue(true);
      }

      continue;
    }

    if (1.1 <= message.httpVersion() &&
        0 == strcasecmp((const char *)header->fieldName(), "Connection")) {
      if (0 == header->fieldValue()) {
        continue;
      }

      if (0 == strncmp((const char *)header->fieldValue(), "close",
                       sizeof("close") - 1)) {
        message.setReuseConnection(false);
      }

      continue;
    }

    if (0 == strcasecmp((const char *)header->fieldName(), "Content-Length")) {
      // 3.If a Content-Length header field (section 14.13) is present, its
      // decimal value in OCTETs represents both the entity-length and the
      // transfer-length. The Content-Length header field MUST NOT be sent
      // if these two lengths are different (i.e., if a Transfer-Encoding
      // header field is present). If a message is received with both a
      // Transfer-Encoding header field and a Content-Length header field,
      // the latter MUST be ignored.

      if (0 == header->fieldValue()) {
        continue;
      }

      error = HttpUtil::ParseInteger(header->fieldValue(), &contentLength);

      if (ES_HTTP_BAD_INTEGER == error) {
        return ES_HTTP_BAD_CONTENT_LENGTH;
      }

      if (ESB_SUCCESS != error) {
        return error;
      }

      foundContentLength = true;

      continue;
    }

    if (0 ==
        strcasecmp((const char *)header->fieldName(), "Transfer-Encoding")) {
      // 2.If a Transfer-Encoding header field (section 14.41) is present and
      // has any value other than "identity", then the transfer-length is
      // defined by use of the "chunked" transfer-coding (section 3.6),
      // unless the message is terminated by closing the connection.

      // Messages MUST NOT include both a Content-Length header field and a
      // non-identity transfer-coding. If the message does include a non-
      // identity transfer-coding, the Content-Length MUST be ignored.

      if (0 == header->fieldValue()) {
        continue;
      }

      if (0 != strncmp((const char *)header->fieldValue(), "identity",
                       sizeof("identity") - 1)) {
        foundTransferEncodedChunked = true;
      }

      continue;
    }

    if (0 == strcasecmp((const char *)header->fieldName(), "Content-Type")) {
      // 4.If the message uses the media type "multipart/byteranges", and the
      // transfer-length is not otherwise specified, then this self-
      // delimiting media type defines the transfer-length. This media type
      // MUST NOT be used unless the sender knows that the recipient can parse
      // it; the presence in a message of a Range header with multiple byte-
      // range specifiers from a 1.1 client implies that the client can parse
      // multipart/byteranges responses.

      if (0 == header->fieldValue()) {
        continue;
      }

      if (0 == strncmp((const char *)header->fieldValue(),
                       "multipart/byteranges",
                       sizeof("multipart/byteranges") - 1)) {
        foundMultipartByteRange = true;
      }

      continue;
    }
  }

  // 5.By the server closing the connection. (Closing the connection
  // cannot be used to indicate the end of a message body, since that
  // would leave no possibility for the server to send back a response.)

  if (foundTransferEncodedChunked) {
    _bodyBytesRemaining = 0;
    _state |= ES_PARSING_CHUNKED_BODY | ES_PARSING_CHUNK_SIZE;

    message.setHasBody(true);
  } else if (foundContentLength) {
    _bodyBytesRemaining = contentLength;
    _state |= ES_PARSING_UNENCODED_BODY;

    message.setHasBody(0 < contentLength);
  } else if (foundMultipartByteRange) {
    _bodyBytesRemaining = 0;
    _state |= ES_PARSING_MULTIPART_BODY;

    message.setHasBody(false);

    // TODO - support multipart byte range  bodies
    return ES_HTTP_MULTIPART_NOT_SUPPORTED;
  } else if (isBodyNotAllowed(message)) {
    // For response messages, whether or not a message-body is included with
    // a message is dependent on both the request method and the response
    // status code (section 6.1.1). All responses to the HEAD request method
    // MUST NOT include a message-body, even though the presence of entity-
    // header fields might lead one to believe they do. All 1xx
    // (informational), 204 (no content), and 304 (not modified) responses
    // MUST NOT include a message-body. All other responses do include a
    // message-body, although it MAY be of zero length

    _bodyBytesRemaining = 0;
    _state |= ES_PARSING_UNENCODED_BODY;

    message.setHasBody(false);
  } else {
    // Just read as much as we can until the socket is closed.
    _bodyBytesRemaining = ESB_UINT64_MAX;
    _state |= ES_PARSING_BODY_UNTIL_CLOSE;

    message.setHasBody(true);
    message.setReuseConnection(false);
  }

  return ESB_SUCCESS;
}

ESB::Error HttpMessageParser::parseVersion(ESB::Buffer *inputBuffer,
                                           HttpMessage &message,
                                           bool clientMode) {
  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

  // Clients SHOULD be tolerant in parsing the Status-Line and servers
  // tolerant when parsing the Request-Line. In particular, they SHOULD
  // accept any amount of SP or HT characters between fields, even though
  // only a single SP is required.

  HttpUtil::SkipSpaces(inputBuffer);

  if (clientMode) {
    if (sizeof("HTTP/1.1") > inputBuffer->readable()) {
      return ESB_AGAIN;
    }
  } else {
    if (sizeof("HTTP/1.1") + 1 > inputBuffer->readable()) {
      return ESB_AGAIN;
    }
  }

  int version;

  if (HttpUtil::IsMatch(inputBuffer, (const unsigned char *)"HTTP/1.1")) {
    inputBuffer->skip(sizeof("HTTP/1.1") - 1);

    version = 110;
  } else if (HttpUtil::IsMatch(inputBuffer,
                               (const unsigned char *)"HTTP/1.0")) {
    inputBuffer->skip(sizeof("HTTP/1.0") - 1);

    version = 100;
  } else {
    return ES_HTTP_BAD_REQUEST_VERSION;
  }

  if (clientMode) {
    if (!HttpUtil::IsSpace(inputBuffer->next())) {
      return ES_HTTP_BAD_REQUEST_VERSION;
    }
  } else {
    switch (inputBuffer->next()) {
      case '\n':

        break;

      case '\r':

        if ('\n' != inputBuffer->next()) {
          return ES_HTTP_BAD_REQUEST_VERSION;
        }

        break;

      default:

        return ES_HTTP_BAD_REQUEST_VERSION;
    }
  }

  message.setHttpVersion(version);

  return ESB_SUCCESS;
}

}  // namespace ES
