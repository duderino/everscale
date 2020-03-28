#ifndef ES_HTTP_RESPONSE_PARSER_H
#define ES_HTTP_RESPONSE_PARSER_H

#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

#ifndef ES_HTTP_MESSAGE_PARSER_H
#include <ESHttpMessageParser.h>
#endif

namespace ES {

/**
 * Parses a HTTP Response as defined in RFC 2616 and RFC 2396
 */
class HttpResponseParser : public HttpMessageParser {
 public:
  /** Create a new response parser
   *
   * @param workingBuffer Temporary storage for parsing
   * @param allocator The discard allocator to use for allocating internal
   * strings.
   */
  HttpResponseParser(ESB::Buffer *workingBuffer,
                     ESB::DiscardAllocator &allocator);

  virtual ~HttpResponseParser();

  /**
   * Reset the parser
   */
  virtual void reset();

 protected:
  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
  virtual ESB::Error parseStartLine(ESB::Buffer *inputBuffer,
                                    HttpMessage &message);

  virtual bool isBodyNotAllowed(HttpMessage &message);

 private:
  // Disabled
  HttpResponseParser(const HttpResponseParser &parser);
  void operator=(const HttpResponseParser &parser);

  // Status-Code    = 3DIGIT
  ESB::Error parseStatusCode(ESB::Buffer *inputBuffer, HttpResponse &response);

  // Reason-Phrase  = *<TEXT, excluding CR, LF>
  ESB::Error parseReasonPhrase(ESB::Buffer *inputBuffer,
                               HttpResponse &response);

  int _responseState;
};

}  // namespace ES

#endif
