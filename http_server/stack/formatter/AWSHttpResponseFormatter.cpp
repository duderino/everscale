/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_RESPONSE_FORMATTER_H
#include <AWSHttpResponseFormatter.h>
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

#define AWS_FORMATTING_VERSION (1 << 0)
#define AWS_FORMATTING_STATUS_CODE (1 << 1)
#define AWS_FORMATTING_REASON_PHRASE (1 << 2)
#define AWS_FORMAT_COMPLETE (1 << 3)

AWSHttpResponseFormatter::AWSHttpResponseFormatter() : _responseState(0x00) {}

AWSHttpResponseFormatter::~AWSHttpResponseFormatter() {}

void AWSHttpResponseFormatter::reset() {
  AWSHttpMessageFormatter::reset();

  _responseState = 0x00;
}

ESFError AWSHttpResponseFormatter::formatStartLine(
    ESFBuffer *outputBuffer, const AWSHttpMessage *message) {
  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF

  if (AWS_FORMAT_COMPLETE & _responseState) {
    return ESF_INVALID_STATE;
  }

  if (0x00 == _responseState) {
    AWSHttpUtil::Start(&_responseState, outputBuffer, AWS_FORMATTING_VERSION);
  }

  AWSHttpResponse *response = (AWSHttpResponse *)message;

  ESFError error = ESF_SUCCESS;

  if (AWS_FORMATTING_VERSION & _responseState) {
    error = formatVersion(outputBuffer, response, false);

    if (ESF_SUCCESS != error) {
      return error;
    }

    AWSHttpUtil::Transition(&_responseState, outputBuffer,
                            AWS_FORMATTING_VERSION, AWS_FORMATTING_STATUS_CODE);
  }

  if (AWS_FORMATTING_STATUS_CODE & _responseState) {
    error = formatStatusCode(outputBuffer, response);

    if (ESF_SUCCESS != error) {
      return error;
    }

    AWSHttpUtil::Transition(&_responseState, outputBuffer,
                            AWS_FORMATTING_STATUS_CODE,
                            AWS_FORMATTING_REASON_PHRASE);
  }

  if (AWS_FORMATTING_REASON_PHRASE & _responseState) {
    error = formatReasonPhrase(outputBuffer, response);

    if (ESF_SUCCESS != error) {
      return error;
    }

    return AWSHttpUtil::Transition(&_responseState, outputBuffer,
                                   AWS_FORMATTING_REASON_PHRASE,
                                   AWS_FORMAT_COMPLETE);
  }

  return ESF_INVALID_STATE;
}

ESFError AWSHttpResponseFormatter::formatStatusCode(
    ESFBuffer *outputBuffer, const AWSHttpResponse *response) {
  // Status-Code    = 3DIGIT

  ESF_ASSERT(AWS_FORMATTING_STATUS_CODE & _responseState);

  if (100 > response->getStatusCode() || 999 < response->getStatusCode()) {
    return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_STATUS_CODE);
  }

  if (4 > outputBuffer->getWritable()) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
  }

  ESFError error =
      AWSHttpUtil::FormatInteger(outputBuffer, response->getStatusCode(), 10);

  if (ESF_SUCCESS != error) {
    return AWSHttpUtil::Rollback(outputBuffer, error);
  }

  if (false == outputBuffer->isWritable()) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
  }

  outputBuffer->putNext(' ');

  return ESF_SUCCESS;
}

ESFError AWSHttpResponseFormatter::formatReasonPhrase(
    ESFBuffer *outputBuffer, const AWSHttpResponse *response) {
  // Reason-Phrase  = *<TEXT, excluding CR, LF>

  ESF_ASSERT(AWS_FORMATTING_REASON_PHRASE & _responseState);

  if (0 == response->getReasonPhrase() || 0 == response->getReasonPhrase()[0]) {
    return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REASON_PHRASE);
  }

  for (const unsigned char *p = response->getReasonPhrase(); *p; ++p) {
    if (false == outputBuffer->isWritable()) {
      return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
    }

    if ('\n' == *p || '\r' == *p) {
      return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REASON_PHRASE);
    }

    if (AWSHttpUtil::IsText(*p)) {
      outputBuffer->putNext(*p);
      continue;
    }

    return AWSHttpUtil::Rollback(outputBuffer, AWS_HTTP_BAD_REASON_PHRASE);
  }

  if (2 > outputBuffer->getWritable()) {
    return AWSHttpUtil::Rollback(outputBuffer, ESF_AGAIN);
  }

  outputBuffer->putNext('\r');
  outputBuffer->putNext('\n');

  return ESF_SUCCESS;
}
