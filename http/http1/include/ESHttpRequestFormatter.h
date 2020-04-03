#ifndef ES_HTTP_REQUEST_FORMATTER_H
#define ES_HTTP_REQUEST_FORMATTER_H

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ES_HTTP_REQUEST_URI_FORMATTER_H
#include <ESHttpRequestUriFormatter.h>
#endif

#ifndef ES_HTTP_MESSAGE_FORMATTER_H
#include <ESHttpMessageFormatter.h>
#endif

namespace ES {

/**
 * Formats a HTTP request as defined in RFC 2616 and RFC 2396
 */
class HttpRequestFormatter : public HttpMessageFormatter {
 public:
  /** Create a new request formatter
   */
  HttpRequestFormatter();

  virtual ~HttpRequestFormatter();

  /**
   * Reset the formatter
   */
  virtual void reset();

 protected:
  // Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
  virtual ESB::Error formatStartLine(ESB::Buffer *outputBuffer,
                                     const HttpMessage &message);

 private:
  // Disabled
  HttpRequestFormatter(const HttpRequestFormatter &formatter);
  void operator=(const HttpRequestFormatter &formatter);

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
  ESB::Error formatMethod(ESB::Buffer *outputBuffer,
                          const HttpRequest &request);

  int _requestState;
  HttpRequestUriFormatter _requestUriFormatter;
  char _pad[16];
};

}  // namespace ES

#endif
