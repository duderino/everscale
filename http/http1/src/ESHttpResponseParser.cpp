#ifndef ES_HTTP_RESPONSE_PARSER_H
#include <ESHttpResponseParser.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

#define ES_PARSING_VERSION (1 << 0)
#define ES_PARSING_STATUS_CODE (1 << 1)
#define ES_PARSING_REASON_PHRASE (1 << 2)
#define ES_PARSE_COMPLETE (1 << 3)

HttpResponseParser::HttpResponseParser(ESB::Buffer *workingBuffer,
                                       ESB::DiscardAllocator &allocator)
    : HttpMessageParser(workingBuffer, allocator), _responseState(0x00) {}

HttpResponseParser::~HttpResponseParser() {}

void HttpResponseParser::reset() {
  HttpMessageParser::reset();

  _responseState = 0x00;
}

ESB::Error HttpResponseParser::parseStartLine(ESB::Buffer *inputBuffer,
                                              HttpMessage &message) {
  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

  if (ES_PARSE_COMPLETE & _responseState) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _responseState) {
    _responseState = ES_PARSING_VERSION;

    inputBuffer->readMark();
    _parseBuffer->clear();
  }

  HttpResponse &response = (HttpResponse &)message;

  ESB::Error error = ESB_SUCCESS;

  if (ES_PARSING_VERSION & _responseState) {
    error = parseVersion(inputBuffer, response, true);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _responseState &= ~ES_PARSING_VERSION;
    _responseState |= ES_PARSING_STATUS_CODE;

    inputBuffer->readMark();
    _parseBuffer->clear();
  }

  if (ES_PARSING_STATUS_CODE & _responseState) {
    error = parseStatusCode(inputBuffer, response);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _responseState &= ~ES_PARSING_STATUS_CODE;
    _responseState |= ES_PARSING_REASON_PHRASE;

    inputBuffer->readMark();
    _parseBuffer->clear();
  }

  if (ES_PARSING_REASON_PHRASE & _responseState) {
    error = parseReasonPhrase(inputBuffer, response);

    if (ESB_SUCCESS != error) {
      return error;
    }

    _responseState &= ~ES_PARSING_REASON_PHRASE;
    _responseState |= ES_PARSE_COMPLETE;

    inputBuffer->readMark();
    _parseBuffer->clear();

    return ESB_SUCCESS;
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpResponseParser::parseStatusCode(ESB::Buffer *inputBuffer,
                                               HttpResponse &response) {
  // Status-Code    = 3DIGIT

  assert(ES_PARSING_STATUS_CODE & _responseState);

  // Clients SHOULD be tolerant in parsing the Status-Line and servers
  // tolerant when parsing the Request-Line. In particular, they SHOULD
  // accept any amount of SP or HT characters between fields, even though
  // only a single SP is required.

  HttpUtil::SkipSpaces(inputBuffer);

  if (4 > inputBuffer->readable()) {
    return ESB_AGAIN;
  }

  int statusCode = 0;
  unsigned char octet;

  for (int i = 0; i < 3; ++i) {
    assert(inputBuffer->isReadable());

    octet = inputBuffer->next();

    if (!HttpUtil::IsDigit(octet)) {
      return ES_HTTP_BAD_STATUS_CODE;
    }

    statusCode = (statusCode * 10) + (octet - '0');
  }

  assert(inputBuffer->isReadable());

  if (!HttpUtil::IsSpace(inputBuffer->next())) {
    return ES_HTTP_BAD_STATUS_CODE;
  }

  response.setStatusCode(statusCode);

  return ESB_SUCCESS;
}

ESB::Error HttpResponseParser::parseReasonPhrase(ESB::Buffer *inputBuffer,
                                                 HttpResponse &response) {
  // Reason-Phrase  = *<TEXT, excluding CR, LF>

  assert(ES_PARSING_REASON_PHRASE & _responseState);

  ESB::Error error;
  unsigned char octet;

  while (true) {
    if (!inputBuffer->isReadable()) {
      return ESB_AGAIN;
    }

    if (!_parseBuffer->isWritable()) {
      return ESB_OVERFLOW;
    }

    octet = inputBuffer->next();

    if (HttpUtil::IsLWS(octet)) {
      inputBuffer->setReadPosition(inputBuffer->readPosition() - 1);

      error = HttpUtil::SkipLWS(inputBuffer);

      switch (error) {
        case ESB_SUCCESS:

          // newline encountered - save reason phrase

          {
            unsigned char *reasonPhrase = _parseBuffer->duplicate(
                _allocator, true);  // trims trailing whitespace

            if (!reasonPhrase) {
              return ESB_OUT_OF_MEMORY;
            }

            response.setReasonPhrase(reasonPhrase);
          }

          return ESB_SUCCESS;

        case ESB_INPROGRESS:

          // LWS encountered - replace with a single space & trim leading white
          // space

          if (0 < _parseBuffer->writePosition()) {
            _parseBuffer->putNext(' ');
          }

          break;

        default:

          return error;
      }

      continue;
    }

    if (HttpUtil::IsText(octet)) {
      _parseBuffer->putNext(octet);
      continue;
    }

    return ES_HTTP_BAD_REASON_PHRASE;
  }
}

bool HttpResponseParser::isBodyNotAllowed(HttpMessage &message) {
  // For response messages, whether or not a message-body is included with
  // a message is dependent on both the request method and the response
  // status code (section 6.1.1). All responses to the HEAD request method
  // MUST NOT include a message-body, even though the presence of entity-
  // header fields might lead one to believe they do. All 1xx
  // (informational), 204 (no content), and 304 (not modified) responses
  // MUST NOT include a message-body. All other responses do include a
  // message-body, although it MAY be of zero length

  HttpResponse &response = (HttpResponse &)message;

  switch (response.statusCode()) {
    case 204:
    case 304:

      return true;

    default:

      return 200 <= response.statusCode();
  }
}

}  // namespace ES
