#ifndef ES_HTTP_RESPONSE_FORMATTER_H
#define ES_HTTP_RESPONSE_FORMATTER_H

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
#endif

#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

#ifndef ES_HTTP_MESSAGE_FORMATTER_H
#include <ESHttpMessageFormatter.h>
#endif

namespace ES {

/**
 * Formats a HTTP response as defined in RFC 2616 and RFC 2396
 */
class HttpResponseFormatter : public HttpMessageFormatter {
 public:
  /** Create a new response formatter
   */
  HttpResponseFormatter();

  virtual ~HttpResponseFormatter();

  /**
   * Reset the formatter
   */
  virtual void reset();

 protected:
  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
  virtual ESB::Error formatStartLine(ESB::Buffer *outputBuffer, const HttpMessage &message);

 private:
  // Status-Code    = 3DIGIT
  ESB::Error formatStatusCode(ESB::Buffer *outputBuffer, const HttpResponse &response);

  // Reason-Phrase  = *<TEXT, excluding CR, LF>
  ESB::Error formatReasonPhrase(ESB::Buffer *outputBuffer, const HttpResponse &response);

  int _responseState;

  ESB_DEFAULT_FUNCS(HttpResponseFormatter);
};

}  // namespace ES

#endif
