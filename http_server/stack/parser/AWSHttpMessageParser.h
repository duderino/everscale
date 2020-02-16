/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_MESSAGE_PARSER_H
#define AWS_HTTP_MESSAGE_PARSER_H

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_BUFFER_H
#include <ESFBuffer.h>
#endif

#ifndef AWS_HTTP_MESSAGE_H
#include <AWSHttpMessage.h>
#endif

/**
 * Parses a HTTP message as defined in RFC 2616 and RFC 2396
 *
 * TODO handle 1.x versions other than 1.0 and 1.1
 */
class AWSHttpMessageParser {
 public:
  /** Create a new message parser
   *
   * @param workingBuffer Temporary storage for parsing
   * @param allocator The discard allocator to use for allocating internal
   * strings.
   */
  AWSHttpMessageParser(ESFBuffer *workingBuffer,
                       ESFDiscardAllocator *allocator);

  virtual ~AWSHttpMessageParser();

  /**
   * Reset the parser
   */
  virtual void reset();

  /**
   * Incrementally parse a HTTP Message up to but not including the body.
   *
   *  generic-message = start-line
   *                    *(message-header CRLF)
   *                    CRLF
   *                    [ message-body ]
   *
   * Caller should:
   * (1) Fill buffer, closing any idle sockets, enforcing any size limits.
   * (2) Parse buffer with this method. If ESF_AGAIN is returned, compact
   *     buffer & mark.  If error or ESF_SUCCESS returned, break.
   * (3) If no space in buffer, parser is jammed, break and close socket.
   * (4) Otherwise goto (1)
   *
   * @param inputBuffer The buffer to parse
   * @param message The http message to build
   * @return ESF_SUCCESS if successful, ESF_AGAIN if more data needs to be read,
   *  another error code otherwise.
   */
  ESFError parseHeaders(ESFBuffer *inputBuffer, AWSHttpMessage *message);

  /**
   * Parse the body.  Caller should keep calling this mehtod on a given input
   * buffer until it returns an error, ESF_AGAIN (buffer needs to be compacted
   * and filled), or the number of bytes to be read is 0 (EOF)
   *
   * @param inputBuffer A buffer full of body data to be parsed.
   * @param startingPosition If ESF_SUCCESS is returned, this will be set to the
   *  starting position of the chunk to be read
   * @param chunkSize If ESF_SUCCESS is returned, this will be set to the number
   * of bytes that can be read after the starting position.  If 0, then there is
   * no more data left in the body.
   * @return ESF_SUCCESS if successful, ESF_AGAIN if the buffer needs to be
   *  compacted and filled, another error code otherwise.
   */
  ESFError parseBody(ESFBuffer *inputBuffer, int *startingPosition,
                     int *chunkSize);

  /**
   * Skips any body trailer.  Necessary only if the connection will be reused.
   *
   * @param inputBuffer A buffer full of data to be parsed
   * @return ESF_SUCCESS if successful, ESF_AGAIN if the buffer needs to be
   * compacted and filled, another error code otherwise.
   */
  ESFError skipTrailer(ESFBuffer *inputBuffer);

 protected:
  virtual ESFError parseStartLine(ESFBuffer *inputBuffer,
                                  AWSHttpMessage *message) = 0;

  virtual bool isBodyNotAllowed(AWSHttpMessage *message) = 0;

  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
  ESFError parseVersion(ESFBuffer *inputBuffer, AWSHttpMessage *message,
                        bool clientMode);

  ESFBuffer *_workingBuffer;
  ESFDiscardAllocator *_allocator;
  int _state;
  ESFUInt64 _bodyBytesRemaining;

 private:
  // Disabled
  AWSHttpMessageParser(const AWSHttpMessageParser &parser);
  void operator=(const AWSHttpMessageParser &parser);

  // field-name     = token
  ESFError parseFieldName(ESFBuffer *inputBuffer, AWSHttpMessage *message);

  // field-value    = *( field-content | LWS )
  // field-content  = <the OCTETs making up the field-value
  //                 and consisting of either *TEXT or combinations
  //                 of token, separators, and quoted-string>
  ESFError parseFieldValue(ESFBuffer *inputBuffer, AWSHttpMessage *message);

  // Chunked-Body   = *chunk
  //                  last-chunk
  //                  trailer
  //                  CRLF
  ESFError parseChunkedBody(ESFBuffer *inputBuffer, int *startingPosition,
                            int *chunkSize);

  // chunk-size     = 1*HEX
  ESFError parseChunkSize(ESFBuffer *inputBuffer);

  // chunk          = ... [ chunk-extension ] CRLF
  // chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
  // chunk-ext-name = token
  // chunk-ext-val  = token | quoted-string
  ESFError parseChunkExtension(ESFBuffer *inputBuffer);

  // chunk-data     = chunk-size(OCTET)
  ESFError parseChunkData(ESFBuffer *inputBuffer, int *startingPosition,
                          int *chunkSize);

  // chunk          = ... CRLF
  ESFError parseEndChunk(ESFBuffer *inputBuffer);

  ESFError parseMultipartBody(ESFBuffer *inputBuffer, int *startingPosition,
                              int *chunkSize);

  ESFError parseUnencodedBody(ESFBuffer *inputBuffer, int *startingPosition,
                              int *chunkSize);

  ESFError postParse(AWSHttpMessage *message);
};

#endif
