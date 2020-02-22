/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_MESSAGE_FORMATTER_H
#define AWS_HTTP_MESSAGE_FORMATTER_H

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

#ifndef AWS_HTTP_MESSAGE_H
#include <AWSHttpMessage.h>
#endif

/**
 * Formats a HTTP message as defined in RFC 2616 and RFC 2396
 */
class AWSHttpMessageFormatter {
 public:
  /** Create a new message formatter
   */
  AWSHttpMessageFormatter();

  virtual ~AWSHttpMessageFormatter();

  /**
   * Reset the formatter
   */
  virtual void reset();

  /**
   * Incrementally format a HTTP Message up to but not including the body.
   *
   *  generic-message = start-line
   *                    *(message-header CRLF)
   *                    CRLF
   *                    [ message-body ]
   *
   * Caller should:
   * (1) Fill buffer with this method.  If return value is not ESF_AGAIN or
   *     ESF_SUCCESS, break
   * (2) Write buffer to socket
   * (3) If ESF_AGAIN was returned, clear buffer & goto (1), otherwise done.
   *
   * @param outputBuffer The buffer to fill
   * @param message The http message to format
   * @return ESF_SUCCESS if successful, ESF_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESFError formatHeaders(ESFBuffer *outputBuffer,
                         const AWSHttpMessage *message);

  /**
   * Format a HTTP body in one or more blocks.  If a Transfer-Encoding 'chunked'
   * header was formatted earlier, chunked transfer encoding will be used.
   * Otherwise no transfer encoding will be used.
   *
   * Caller should:
   * (1) Format buffer with this method.  If return value is not ESF_AGAIN or
   *     ESF_SUCCESS, break
   * (2) If ESF_AGAIN was returned, write buffer to socket, clear buffer & goto
   * (1) (3) Otherwise write availableSize bytes directly to the buffer - this
   * function does not actually copy the body data into the buffer. (4) Call
   * endBlock.  If return value is not ESF_AGAIN or ESF_SUCCESS, break. (5) If
   * ESF_AGAIN was returned, write buffer to socket, clear buffer & goto (4) (6)
   * If availableSize == requestedSize & there is more body data, goto (1) (7)
   * If availableSize < requestedSize, the buffer is full.  write buffer to
   * socket, clear buffer & goto (1)
   *
   * @param outputBuffer The buffer to fill
   * @param requestedSize The number of bytes the caller would like to add to
   * the body
   * @param availableSize The number of bytes that can be added to the
   * outputBuffer after this call returns.  Always less than or equal to
   * requestedSize.
   * @return ESF_SUCCESS if successful, ESF_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESFError beginBlock(ESFBuffer *outputBuffer, int requestedSize,
                      int *availableSize);

  ESFError endBlock(ESFBuffer *outputBuffer);

  /**
   * End the HTTP body.
   *
   * Caller should:
   * (1) Fill buffer with this method.  If return value is not ESF_AGAIN or
   *     ESF_SUCCESS, break
   * (2) Write buffer to socket
   * (3) If ESF_AGAIN was returned, clear buffer & goto (1), otherwise done.
   *
   * @param outputBuffer The buffer to fill
   * @return ESF_SUCCESS if successful, ESF_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESFError endBody(ESFBuffer *outputBuffer);

 protected:
  virtual ESFError formatStartLine(ESFBuffer *outputBuffer,
                                   const AWSHttpMessage *message) = 0;

  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
  ESFError formatVersion(ESFBuffer *outputBuffer, const AWSHttpMessage *message,
                         bool clientMode);

 private:
  // Disabled
  AWSHttpMessageFormatter(const AWSHttpMessageFormatter &formatter);
  void operator=(const AWSHttpMessageFormatter &formatter);

  // field-name     = token
  ESFError formatFieldName(ESFBuffer *outputBuffer,
                           const unsigned char *fieldName);

  // field-value    = *( field-content | LWS )
  // field-content  = <the OCTETs making up the field-value
  //                 and consisting of either *TEXT or combinations
  //                 of token, separators, and quoted-string>
  ESFError formatFieldValue(ESFBuffer *outputBuffer,
                            const unsigned char *fieldValue);

  // chunk          = chunk-size [ chunk-extension ] CRLF
  //                  ...
  // chunk-size     = 1*HEX
  ESFError beginChunk(ESFBuffer *outputBuffer, int requestedSize,
                      int *availableSize);

  // chunk          = ...
  //                  chunk-data CRLF
  ESFError endChunk(ESFBuffer *outputBuffer);

  ESFError beginUnencodedBlock(ESFBuffer *outputBuffer, int requestedSize,
                               int *availableSize);

  int _state;
  const AWSHttpHeader *_currentHeader;
};

#endif
