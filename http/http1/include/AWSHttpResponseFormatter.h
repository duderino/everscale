/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_RESPONSE_FORMATTER_H
#define AWS_HTTP_RESPONSE_FORMATTER_H

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

#ifndef AWS_HTTP_RESPONSE_H
#include <AWSHttpResponse.h>
#endif

#ifndef AWS_HTTP_MESSAGE_FORMATTER_H
#include <AWSHttpMessageFormatter.h>
#endif

/**
 * Formats a HTTP response as defined in RFC 2616 and RFC 2396
 */
class AWSHttpResponseFormatter : public AWSHttpMessageFormatter {
 public:
  /** Create a new response formatter
   */
  AWSHttpResponseFormatter();

  virtual ~AWSHttpResponseFormatter();

  /**
   * Reset the formatter
   */
  virtual void reset();

 protected:
  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
  virtual ESFError formatStartLine(ESFBuffer *outputBuffer,
                                   const AWSHttpMessage *message);

 private:
  // Disabled
  AWSHttpResponseFormatter(const AWSHttpResponseFormatter &formatter);
  void operator=(const AWSHttpResponseFormatter &formatter);

  // Status-Code    = 3DIGIT
  ESFError formatStatusCode(ESFBuffer *outputBuffer,
                            const AWSHttpResponse *response);

  // Reason-Phrase  = *<TEXT, excluding CR, LF>
  ESFError formatReasonPhrase(ESFBuffer *outputBuffer,
                              const AWSHttpResponse *response);

  int _responseState;
};

#endif
