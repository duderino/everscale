#ifndef ES_HTTP_MESSAGE_PARSER_H
#define ES_HTTP_MESSAGE_PARSER_H

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ES_HTTP_MESSAGE_H
#include <ESHttpMessage.h>
#endif

namespace ES {

/**
 * Parses a HTTP message as defined in RFC 2616 and RFC 2396
 *
 * TODO handle 1.x versions other than 1.0 and 1.1
 */
class HttpMessageParser {
 public:
  /** Create a new message parser
   *
   * @param workingBuffer Temporary storage for parsing
   * @param allocator The discard allocator to use for allocating internal
   * strings.
   */
  HttpMessageParser(ESB::Buffer *workingBuffer,
                    ESB::DiscardAllocator &allocator);

  virtual ~HttpMessageParser();

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
   * (2) Parse buffer with this method. If ESB_AGAIN is returned, compact
   *     buffer & mark.  If error or ESB_SUCCESS returned, break.
   * (3) If no space in buffer, parser is jammed, break and close socket.
   * (4) Otherwise goto (1)
   *
   * @param inputBuffer The buffer to parse
   * @param message The http message to build
   * @return ESB_SUCCESS if successful, ESB_AGAIN if more data needs to be read,
   *  another error code otherwise.
   */
  ESB::Error parseHeaders(ESB::Buffer *inputBuffer, HttpMessage &message);

  /**
   * Parse the body.  Caller should keep calling this mehtod on a given input
   * buffer until it returns an error, ESB_AGAIN (buffer needs to be compacted
   * and filled), or the number of bytes to be read is 0 (EOF)
   *
   * @param inputBuffer A buffer full of body data to be parsed.
   * @param startingPosition If ESB_SUCCESS is returned, this will be set to the
   *  starting position of the chunk to be read
   * @param chunkSize If ESB_SUCCESS is returned, this will be set to the number
   * of bytes that can be read after the starting position.  If 0, then there is
   * no more data left in the body.
   * @param maxChunkSize but read no more than this even if data is available.
   * @return ESB_SUCCESS if successful, ESB_AGAIN if the buffer needs to be
   *  compacted and filled, another error code otherwise.
   */
  ESB::Error parseBody(ESB::Buffer *inputBuffer, ESB::UInt32 *startingPosition,
                       ESB::UInt32 *chunkSize, ESB::UInt32 maxChunkSize);

  /**
   * Skips any body trailer.  Necessary only if the connection will be reused.
   *
   * @param inputBuffer A buffer full of data to be parsed
   * @return ESB_SUCCESS if successful, ESB_AGAIN if the buffer needs to be
   * compacted and filled, another error code otherwise.
   */
  ESB::Error skipTrailer(ESB::Buffer *inputBuffer);

 protected:
  virtual ESB::Error parseStartLine(ESB::Buffer *inputBuffer,
                                    HttpMessage &message) = 0;

  virtual bool isBodyNotAllowed(HttpMessage &message) = 0;

  // HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
  ESB::Error parseVersion(ESB::Buffer *inputBuffer, HttpMessage &message,
                          bool clientMode);

  ESB::Buffer *_workingBuffer;
  ESB::DiscardAllocator &_allocator;
  int _state;
  ESB::UInt64 _bodyBytesRemaining;

 private:
  // Disabled
  HttpMessageParser(const HttpMessageParser &parser);
  void operator=(const HttpMessageParser &parser);

  // field-name     = token
  ESB::Error parseFieldName(ESB::Buffer *inputBuffer, HttpMessage &message);

  // field-value    = *( field-content | LWS )
  // field-content  = <the OCTETs making up the field-value
  //                 and consisting of either *TEXT or combinations
  //                 of token, separators, and quoted-string>
  ESB::Error parseFieldValue(ESB::Buffer *inputBuffer, HttpMessage &message);

  // Chunked-Body   = *chunk
  //                  last-chunk
  //                  trailer
  //                  CRLF
  ESB::Error parseChunkedBody(ESB::Buffer *inputBuffer,
                              ESB::UInt32 *startingPosition,
                              ESB::UInt32 *chunkSize, ESB::UInt32 maxChunkSize);

  // chunk-size     = 1*HEX
  ESB::Error parseChunkSize(ESB::Buffer *inputBuffer);

  // chunk          = ... [ chunk-extension ] CRLF
  // chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
  // chunk-ext-name = token
  // chunk-ext-val  = token | quoted-string
  ESB::Error parseChunkExtension(ESB::Buffer *inputBuffer);

  // chunk-data     = chunk-size(OCTET)
  ESB::Error parseChunkData(ESB::Buffer *inputBuffer,
                            ESB::UInt32 *startingPosition,
                            ESB::UInt32 *chunkSize, ESB::UInt32 maxChunkSize);

  // chunk          = ... CRLF
  ESB::Error parseEndChunk(ESB::Buffer *inputBuffer);

  ESB::Error parseMultipartBody(ESB::Buffer *inputBuffer,
                                ESB::UInt32 *startingPosition,
                                ESB::UInt32 *chunkSize);

  ESB::Error parseUnencodedBody(ESB::Buffer *inputBuffer,
                                ESB::UInt32 *startingPosition,
                                ESB::UInt32 *chunkSize);

  ESB::Error postParse(HttpMessage &message);
};

}  // namespace ES

#endif
