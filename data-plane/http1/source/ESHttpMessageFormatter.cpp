#ifndef ES_HTTP_MESSAGE_FORMATTER_H
#include <ESHttpMessageFormatter.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

namespace ES {

#define ES_FORMATTING_START_LINE (1 << 0)
#define ES_FORMATTING_FIELD_NAME (1 << 1)
#define ES_FORMATTING_FIELD_VALUE (1 << 2)
#define ES_HEADER_FORMAT_COMPLETE (1 << 3)
#define ES_FORMATTING_UNENCODED_BODY (1 << 4)
#define ES_FORMATTING_CHUNKED_BODY (1 << 5)
#define ES_BODY_FORMAT_COMPLETE (1 << 6)
#define ES_FORMATTING_HEADER (ES_FORMATTING_FIELD_NAME | ES_FORMATTING_FIELD_VALUE)
#define ES_FOUND_CONTENT_LENGTH_HEADER (1 << 10)
#define ES_FOUND_TRANSFER_ENCODING_CHUNKED_HEADER (1 << 11)

HttpMessageFormatter::HttpMessageFormatter() : _state(0x00), _currentHeader(0) {}

HttpMessageFormatter::~HttpMessageFormatter() {}

void HttpMessageFormatter::reset() {
  _state = 0x00;
  _currentHeader = 0;
}

ESB::Error HttpMessageFormatter::formatHeaders(ESB::Buffer *outputBuffer, const HttpMessage &message) {
  // generic-message = start-line
  //                   *(message-header CRLF)
  //                   CRLF
  //                   [ message-body ]

  if (ES_HEADER_FORMAT_COMPLETE & _state) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _state) {
    if (!outputBuffer->isWritable()) {
      return ESB_AGAIN;
    }

    HttpUtil::Start(&_state, outputBuffer, ES_FORMATTING_START_LINE);
  }

  ESB::Error error = ESB_SUCCESS;

  if (ES_FORMATTING_START_LINE & _state) {
    error = formatStartLine(outputBuffer, message);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _currentHeader = (HttpHeader *)message.headers().first();

    HttpUtil::Transition(&_state, outputBuffer, ES_FORMATTING_START_LINE, ES_FORMATTING_FIELD_NAME);
  }

  while (ES_FORMATTING_HEADER & _state) {
    if (0 == _currentHeader) {
      assert(ES_FORMATTING_FIELD_NAME & _state);

      if (2 > outputBuffer->writable()) {
        outputBuffer->writeReset();
        return ESB_AGAIN;
      }

      outputBuffer->putNext('\r');
      outputBuffer->putNext('\n');

      // Messages MUST NOT include both a Content-Length header field and a
      // non-identity transfer-coding. If the message does include a non-
      // identity transfer-coding, the Content-Length MUST be ignored.

      if (ES_FOUND_TRANSFER_ENCODING_CHUNKED_HEADER & _state) {
        return HttpUtil::Transition(&_state, outputBuffer, ES_FORMATTING_FIELD_NAME,
                                    ES_HEADER_FORMAT_COMPLETE | ES_FORMATTING_CHUNKED_BODY);
      } else {
        return HttpUtil::Transition(&_state, outputBuffer, ES_FORMATTING_FIELD_NAME,
                                    ES_HEADER_FORMAT_COMPLETE | ES_FORMATTING_UNENCODED_BODY);
      }
    }

    if (ES_FORMATTING_FIELD_NAME & _state) {
      assert(_currentHeader->fieldName());

      if (0 == _currentHeader->fieldName()) {
        return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_FIELD_NAME);
      }

      error = formatFieldName(outputBuffer, _currentHeader->fieldName());

      if (ESB_SUCCESS != error) {
        return error;
      }

      if (0 == strcasecmp((const char *)_currentHeader->fieldName(), "Content-Length")) {
        _state |= ES_FOUND_CONTENT_LENGTH_HEADER;
      }

      if (0 == strcasecmp((const char *)_currentHeader->fieldName(), "Transfer-Encoding")) {
        // If a Transfer-Encoding header field (section 14.41) is present and
        // has any value other than "identity", then the transfer-length is
        // defined by use of the "chunked" transfer-coding (section 3.6),
        // unless the message is terminated by closing the connection.

        if (0 == _currentHeader->fieldValue() ||
            0 != strncmp((const char *)_currentHeader->fieldValue(), "identity", sizeof("identity") - 1)) {
          _state |= ES_FOUND_TRANSFER_ENCODING_CHUNKED_HEADER;
        }
      }

      HttpUtil::Transition(&_state, outputBuffer, ES_FORMATTING_FIELD_NAME, ES_FORMATTING_FIELD_VALUE);
    }

    if (ES_FORMATTING_FIELD_VALUE & _state) {
      error = formatFieldValue(outputBuffer, _currentHeader->fieldValue());

      if (ESB_SUCCESS != error) {
        return error;
      }

      HttpUtil::Transition(&_state, outputBuffer, ES_FORMATTING_FIELD_VALUE, ES_FORMATTING_FIELD_NAME);

      _currentHeader = (const HttpHeader *)_currentHeader->next();
    }
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpMessageFormatter::formatVersion(ESB::Buffer *outputBuffer, const HttpMessage &message, bool clientMode) {
  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT

  if (clientMode) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext(' ');
  }

  const unsigned char *version = 0;

  if (110 == message.httpVersion()) {
    version = (const unsigned char *)"HTTP/1.1";
  } else if (100 == message.httpVersion()) {
    version = (const unsigned char *)"HTTP/1.0";
  } else {
    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_VERSION);
  }

  for (const unsigned char *p = version; *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext(*p);
  }

  if (clientMode) {
    if (2 > outputBuffer->writable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext('\r');
    outputBuffer->putNext('\n');
  } else {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    outputBuffer->putNext(' ');
  }

  return ESB_SUCCESS;
}

ESB::Error HttpMessageFormatter::formatFieldName(ESB::Buffer *outputBuffer, const unsigned char *fieldName) {
  // field-name     = token

  assert(ES_FORMATTING_FIELD_NAME & _state);

  for (const unsigned char *p = (const unsigned char *)fieldName; *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    if (HttpUtil::IsToken(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_FIELD_NAME);
  }

  if (2 > outputBuffer->writable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext(':');
  outputBuffer->putNext(' ');

  return ESB_SUCCESS;
}

ESB::Error HttpMessageFormatter::formatFieldValue(ESB::Buffer *outputBuffer, const unsigned char *fieldValue) {
  // field-value    = *( field-content | LWS )
  // field-content  = <the OCTETs making up the field-value
  //                 and consisting of either *TEXT or combinations
  //                 of token, separators, and quoted-string>

  assert(ES_FORMATTING_FIELD_VALUE & _state);

  bool lastIsSpace = true;

  for (const unsigned char *p = (const unsigned char *)fieldValue; *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    if (HttpUtil::IsLWS(*p)) {
      // Replace any line continuations, etc. with a single ' '

      if (!lastIsSpace) {
        outputBuffer->putNext(' ');
        lastIsSpace = true;
      }

      continue;
    }

    lastIsSpace = false;

    if (HttpUtil::IsToken(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    if (HttpUtil::IsSeparator(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    if (HttpUtil::IsText(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REQUEST_FIELD_VALUE);
  }

  if (2 > outputBuffer->writable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext('\r');
  outputBuffer->putNext('\n');

  return ESB_SUCCESS;
}

ESB::Error HttpMessageFormatter::beginBlock(ESB::Buffer *outputBuffer, ESB::UInt32 offeredSize,
                                            ESB::UInt32 *maxChunkSize) {
  if (!outputBuffer || !maxChunkSize) {
    return ESB_NULL_POINTER;
  }

  if (0 == offeredSize) {
    return ESB_INVALID_ARGUMENT;
  }

  if (ES_BODY_FORMAT_COMPLETE & _state) {
    return ESB_INVALID_STATE;
  }

  if (ES_FORMATTING_CHUNKED_BODY & _state) {
    return beginChunk(outputBuffer, offeredSize, maxChunkSize);
  }

  if (ES_FORMATTING_UNENCODED_BODY & _state) {
    return beginUnencodedBlock(outputBuffer, offeredSize, maxChunkSize);
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpMessageFormatter::endBlock(ESB::Buffer *outputBuffer) {
  if (!outputBuffer) {
    return ESB_NULL_POINTER;
  }

  if (ES_BODY_FORMAT_COMPLETE & _state) {
    return ESB_INVALID_STATE;
  }

  if (ES_FORMATTING_CHUNKED_BODY & _state) {
    return endChunk(outputBuffer);
  }

  if (ES_FORMATTING_UNENCODED_BODY & _state) {
    return ESB_SUCCESS;
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpMessageFormatter::endBody(ESB::Buffer *outputBuffer) {
  if (!outputBuffer) {
    return ESB_NULL_POINTER;
  }

  if (ES_BODY_FORMAT_COMPLETE & _state) {
    return ESB_INVALID_STATE;
  }

  if (ES_FORMATTING_CHUNKED_BODY & _state) {
    // Chunked-Body   = ...
    //                  last-chunk
    //                  CRLF
    // last-chunk     = 1*("0") [ chunk-extension ] CRLF

    if (5 > outputBuffer->writable()) {
      return ESB_AGAIN;
    }

    outputBuffer->putNext('0');
    outputBuffer->putNext('\r');
    outputBuffer->putNext('\n');
    outputBuffer->putNext('\r');
    outputBuffer->putNext('\n');

    return HttpUtil::Transition(&_state, outputBuffer, ES_FORMATTING_CHUNKED_BODY, ES_BODY_FORMAT_COMPLETE);
  }

  if (ES_FORMATTING_UNENCODED_BODY & _state) {
    return HttpUtil::Transition(&_state, outputBuffer, ES_FORMATTING_UNENCODED_BODY, ES_BODY_FORMAT_COMPLETE);
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpMessageFormatter::beginChunk(ESB::Buffer *outputBuffer, ESB::UInt32 requestedSize,
                                            ESB::UInt32 *availableSize) {
  // chunk          = chunk-size [ chunk-extension ] CRLF
  //                  ...
  // chunk-size     = 1*HEX

  assert(ES_FORMATTING_CHUNKED_BODY & _state);
  assert(0 < requestedSize);
  assert(availableSize);

  // reserve characters for chunk-size and the CRLF after the chunk data
  // Max supported chunk-size is 0xFFFFFFFF.  So reserve 8 for chunk-size
  // + 2 for the CRLF in the chunk-size production + 2 for the CRLF after
  // the chunk data.

  ESB::Int64 outputBufferSpace = MAX(((ESB::Int64)outputBuffer->writable()) - 12, 0);

  if (0 >= outputBufferSpace) {
    return ESB_AGAIN;
  }

  *availableSize = MIN(requestedSize, outputBufferSpace);

  assert(0 < *availableSize);

  ESB::Error error = HttpUtil::FormatInteger(outputBuffer, *availableSize, 16);

  if (ESB_SUCCESS != error) {
    return HttpUtil::Rollback(outputBuffer, error);
  }

  if (2 > outputBuffer->writable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext('\r');
  outputBuffer->putNext('\n');

  return ESB_SUCCESS;
}

ESB::Error HttpMessageFormatter::endChunk(ESB::Buffer *outputBuffer) {
  // chunk          = ...
  //                  chunk-data CRLF
  // chunk-data     = chunk-size(OCTET)

  assert(ES_FORMATTING_CHUNKED_BODY & _state);

  if (2 > outputBuffer->writable()) {
    return ESB_AGAIN;
  }

  outputBuffer->putNext('\r');
  outputBuffer->putNext('\n');

  return ESB_SUCCESS;
}

ESB::Error HttpMessageFormatter::beginUnencodedBlock(ESB::Buffer *outputBuffer, ESB::UInt32 requestedSize,
                                                     ESB::UInt32 *availableSize) {
  assert(ES_FORMATTING_UNENCODED_BODY & _state);
  assert(0 < requestedSize);
  assert(availableSize);

  if (0 >= outputBuffer->writable()) {
    return ESB_AGAIN;
  }

  *availableSize = MIN(requestedSize, outputBuffer->writable());

  return ESB_SUCCESS;
}

}  // namespace ES
