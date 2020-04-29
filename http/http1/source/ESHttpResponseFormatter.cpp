#ifndef ES_HTTP_RESPONSE_FORMATTER_H
#include <ESHttpResponseFormatter.h>
#endif

#ifndef ES_HTTP_UTIL_H
#include <ESHttpUtil.h>
#endif

#ifndef ES_HTTP_ERROR_H
#include <ESHttpError.h>
#endif

namespace ES {

#define ES_FORMATTING_VERSION (1 << 0)
#define ES_FORMATTING_STATUS_CODE (1 << 1)
#define ES_FORMATTING_REASON_PHRASE (1 << 2)
#define ES_FORMAT_COMPLETE (1 << 3)

HttpResponseFormatter::HttpResponseFormatter() : _responseState(0x00) {}

HttpResponseFormatter::~HttpResponseFormatter() {}

void HttpResponseFormatter::reset() {
  HttpMessageFormatter::reset();

  _responseState = 0x00;
}

ESB::Error HttpResponseFormatter::formatStartLine(ESB::Buffer *outputBuffer,
                                                  const HttpMessage &message) {
  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

  if (ES_FORMAT_COMPLETE & _responseState) {
    return ESB_INVALID_STATE;
  }

  if (0x00 == _responseState) {
    HttpUtil::Start(&_responseState, outputBuffer, ES_FORMATTING_VERSION);
  }

  HttpResponse &response = (HttpResponse &)message;

  ESB::Error error = ESB_SUCCESS;

  if (ES_FORMATTING_VERSION & _responseState) {
    error = formatVersion(outputBuffer, response, false);

    if (ESB_SUCCESS != error) {
      return error;
    }

    HttpUtil::Transition(&_responseState, outputBuffer, ES_FORMATTING_VERSION,
                         ES_FORMATTING_STATUS_CODE);
  }

  if (ES_FORMATTING_STATUS_CODE & _responseState) {
    error = formatStatusCode(outputBuffer, response);

    if (ESB_SUCCESS != error) {
      return error;
    }

    HttpUtil::Transition(&_responseState, outputBuffer,
                         ES_FORMATTING_STATUS_CODE,
                         ES_FORMATTING_REASON_PHRASE);
  }

  if (ES_FORMATTING_REASON_PHRASE & _responseState) {
    error = formatReasonPhrase(outputBuffer, response);

    if (ESB_SUCCESS != error) {
      return error;
    }

    return HttpUtil::Transition(&_responseState, outputBuffer,
                                ES_FORMATTING_REASON_PHRASE,
                                ES_FORMAT_COMPLETE);
  }

  return ESB_INVALID_STATE;
}

ESB::Error HttpResponseFormatter::formatStatusCode(
    ESB::Buffer *outputBuffer, const HttpResponse &response) {
  // Status-Code    = 3DIGIT

  assert(ES_FORMATTING_STATUS_CODE & _responseState);

  if (100 > response.statusCode() || 999 < response.statusCode()) {
    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_STATUS_CODE);
  }

  if (4 > outputBuffer->writable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  ESB::Error error =
      HttpUtil::FormatInteger(outputBuffer, response.statusCode(), 10);

  if (ESB_SUCCESS != error) {
    return HttpUtil::Rollback(outputBuffer, error);
  }

  if (!outputBuffer->isWritable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext(' ');

  return ESB_SUCCESS;
}

ESB::Error HttpResponseFormatter::formatReasonPhrase(
    ESB::Buffer *outputBuffer, const HttpResponse &response) {
  // Reason-Phrase  = *<TEXT, excluding CR, LF>

  assert(ES_FORMATTING_REASON_PHRASE & _responseState);

  if (0 == response.reasonPhrase() || 0 == response.reasonPhrase()[0]) {
    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REASON_PHRASE);
  }

  for (const unsigned char *p = response.reasonPhrase(); *p; ++p) {
    if (!outputBuffer->isWritable()) {
      return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
    }

    if ('\n' == *p || '\r' == *p) {
      return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REASON_PHRASE);
    }

    if (HttpUtil::IsText(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    return HttpUtil::Rollback(outputBuffer, ES_HTTP_BAD_REASON_PHRASE);
  }

  if (2 > outputBuffer->writable()) {
    return HttpUtil::Rollback(outputBuffer, ESB_AGAIN);
  }

  outputBuffer->putNext('\r');
  outputBuffer->putNext('\n');

  return ESB_SUCCESS;
}

}  // namespace ES
