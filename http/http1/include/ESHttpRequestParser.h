#ifndef ES_HTTP_REQUEST_PARSER_H
#define ES_HTTP_REQUEST_PARSER_H

#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ES_HTTP_REQUEST_URI_PARSER_H
#include <ESHttpRequestUriParser.h>
#endif

#ifndef ES_HTTP_MESSAGE_PARSER_H
#include <ESHttpMessageParser.h>
#endif

namespace ES {

/**
 * Parses a HTTP Request as defined in RFC 2616 and RFC 2396
 *
 */
class HttpRequestParser : public HttpMessageParser {
 public:
  /** Create a new request parser
   *
   * @param workingBuffer Temporary storage for parsing
   * @param allocator The discard allocator to use for allocating internal
   * strings.
   */
  HttpRequestParser(ESB::Buffer *workingBuffer,
                    ESB::DiscardAllocator *allocator);

  virtual ~HttpRequestParser();

  /**
   * Reset the parser
   */
  virtual void reset();

 protected:
  // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
  virtual ESB::Error parseStartLine(ESB::Buffer *inputBuffer,
                                    HttpMessage *message);

  virtual bool isBodyNotAllowed(HttpMessage *message);

 private:
  // Disabled
  HttpRequestParser(const HttpRequestParser &parser);
  void operator=(const HttpRequestParser &parser);

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
  ESB::Error parseMethod(ESB::Buffer *inputBuffer, HttpRequest *request);

  int _requestState;
  HttpRequestUriParser _requestUriParser;
};

}  // namespace ES

#endif
