#ifndef ES_HTTP_MESSAGE_FORMATTER_H
#define ES_HTTP_MESSAGE_FORMATTER_H

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ES_HTTP_MESSAGE_H
#include <ESHttpMessage.h>
#endif

namespace ES {

/**
 * Formats a HTTP message as defined in RFC 2616 and RFC 2396
 */
class HttpMessageFormatter {
 public:
  /** Create a new message formatter
   */
  HttpMessageFormatter();

  virtual ~HttpMessageFormatter();

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
   * (1) Fill buffer with this method.  If return value is not ESB_AGAIN or
   *     ESB_SUCCESS, break
   * (2) Write buffer to socket
   * (3) If ESB_AGAIN was returned, clear buffer & goto (1), otherwise done.
   *
   * @param outputBuffer The buffer to fill
   * @param message The http message to format
   * @return ESB_SUCCESS if successful, ESB_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESB::Error formatHeaders(ESB::Buffer *outputBuffer, const HttpMessage &message);

  /**
   * Format a HTTP body in one or more blocks.  If a Transfer-Encoding 'chunked'
   * header was formatted earlier, chunked transfer encoding will be used.
   * Otherwise no transfer encoding will be used.
   *
   * Caller should:
   * (1) Format buffer with this method.  If return value is not ESB_AGAIN or
   *     ESB_SUCCESS, break
   * (2) If ESB_AGAIN was returned, write buffer to socket, clear buffer & goto
   * (1) (3) Otherwise write availableSize bytes directly to the buffer - this
   * function does not actually copy the body data into the buffer. (4) Call
   * endBlock.  If return value is not ESB_AGAIN or ESB_SUCCESS, break. (5) If
   * ESB_AGAIN was returned, write buffer to socket, clear buffer & goto (4) (6)
   * If availableSize == requestedSize & there is more body data, goto (1) (7)
   * If availableSize < requestedSize, the buffer is full.  write buffer to
   * socket, clear buffer & goto (1)
   *
   * @param outputBuffer The buffer to fill
   * @param offeredSize The number of bytes the caller would like to add to
   * the body
   * @param maxChunkSize The number of bytes that can be added to the
   * outputBuffer after this call returns.  Always less than or equal to
   * offeredSize.
   * @return ESB_SUCCESS if successful, ESB_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESB::Error beginBlock(ESB::Buffer *outputBuffer, ESB::UInt32 offeredSize, ESB::UInt32 *maxChunkSize);

  ESB::Error endBlock(ESB::Buffer *outputBuffer);

  /**
   * End the HTTP body.
   *
   * Caller should:
   * (1) Fill buffer with this method.  If return value is not ESB_AGAIN or
   *     ESB_SUCCESS, break
   * (2) Write buffer to socket
   * (3) If ESB_AGAIN was returned, clear buffer & goto (1), otherwise done.
   *
   * @param outputBuffer The buffer to fill
   * @return ESB_SUCCESS if successful, ESB_AGAIN if the buffer runs out of
   * space, another error code otherwise.
   */
  ESB::Error endBody(ESB::Buffer *outputBuffer);

 protected:
  virtual ESB::Error formatStartLine(ESB::Buffer *outputBuffer, const HttpMessage &message) = 0;

  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
  ESB::Error formatVersion(ESB::Buffer *outputBuffer, const HttpMessage &message, bool clientMode);

 private:
  // Disabled
  HttpMessageFormatter(const HttpMessageFormatter &formatter);
  void operator=(const HttpMessageFormatter &formatter);

  // field-name     = token
  inline ESB::Error formatFieldName(ESB::Buffer *outputBuffer, const char *fieldName) {
    return formatFieldName(outputBuffer, (const unsigned char *)fieldName);
  }

  ESB::Error formatFieldName(ESB::Buffer *outputBuffer, const unsigned char *fieldName);

  // field-value    = *( field-content | LWS )
  // field-content  = <the OCTETs making up the field-value
  //                 and consisting of either *TEXT or combinations
  //                 of token, separators, and quoted-string>
  inline ESB::Error formatFieldValue(ESB::Buffer *outputBuffer, const char *fieldValue) {
    return formatFieldValue(outputBuffer, (const unsigned char *)fieldValue);
  }

  ESB::Error formatFieldValue(ESB::Buffer *outputBuffer, const unsigned char *fieldValue);

  // chunk          = chunk-size [ chunk-extension ] CRLF
  //                  ...
  // chunk-size     = 1*HEX
  ESB::Error beginChunk(ESB::Buffer *outputBuffer, ESB::UInt32 requestedSize, ESB::UInt32 *availableSize);

  // chunk          = ...
  //                  chunk-data CRLF
  ESB::Error endChunk(ESB::Buffer *outputBuffer);

  ESB::Error beginUnencodedBlock(ESB::Buffer *outputBuffer, ESB::UInt32 requestedSize, ESB::UInt32 *availableSize);

  int _state;
  const HttpHeader *_currentHeader;
};

}  // namespace ES

#endif
