/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_REQUEST_PARSER_H
#define AWS_HTTP_REQUEST_PARSER_H

#ifndef AWS_HTTP_REQUEST_H
#include <AWSHttpRequest.h>
#endif

#ifndef AWS_HTTP_REQUEST_URI_PARSER_H
#include <AWSHttpRequestUriParser.h>
#endif

#ifndef AWS_HTTP_MESSAGE_PARSER_H
#include <AWSHttpMessageParser.h>
#endif

/**
 * Parses a HTTP Request as defined in RFC 2616 and RFC 2396
 *
 */
class AWSHttpRequestParser : public AWSHttpMessageParser {
 public:
  /** Create a new request parser
   *
   * @param workingBuffer Temporary storage for parsing
   * @param allocator The discard allocator to use for allocating internal
   * strings.
   */
  AWSHttpRequestParser(ESFBuffer *workingBuffer,
                       ESFDiscardAllocator *allocator);

  virtual ~AWSHttpRequestParser();

  /**
   * Reset the parser
   */
  virtual void reset();

 protected:
  // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
  virtual ESFError parseStartLine(ESFBuffer *inputBuffer,
                                  AWSHttpMessage *message);

  virtual bool isBodyNotAllowed(AWSHttpMessage *message);

 private:
  // Disabled
  AWSHttpRequestParser(const AWSHttpRequestParser &parser);
  void operator=(const AWSHttpRequestParser &parser);

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
  ESFError parseMethod(ESFBuffer *inputBuffer, AWSHttpRequest *request);

  int _requestState;
  AWSHttpRequestUriParser _requestUriParser;
};

#endif
