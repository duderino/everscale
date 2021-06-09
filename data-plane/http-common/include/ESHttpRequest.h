#ifndef ES_HTTP_REQUEST_H
#define ES_HTTP_REQUEST_H

#ifndef ES_HTTP_REQUEST_URI_H
#include <ESHttpRequestUri.h>
#endif

#ifndef ES_HTTP_MESSAGE_H
#include <ESHttpMessage.h>
#endif

namespace ES {

/**
 * A HTTP Request as defined in RFC 2616 and RFC 2396
 */
class HttpRequest : public HttpMessage {
 public:
  HttpRequest();

  virtual ~HttpRequest();

  void reset();

  ESB::Error copy(const HttpRequest *other, ESB::Allocator &allocator);

  inline const unsigned char *method() const { return _method; }

  inline void setMethod(const unsigned char *method) { _method = method; }
  inline void setMethod(const char *method) { _method = (const unsigned char *)method; }

  inline HttpRequestUri &requestUri() { return _requestUri; }

  inline const HttpRequestUri &requestUri() const { return _requestUri; }

  /**
   * Parse the request uri and any Host header to determine the (unresolved)
   * address where this request should be sent to / was sent to.
   *
   * @param hostname A buffer to store the hostname
   * @param size The size of the hostname buffer
   * @param port A short to store the port
   * @param isSecure HTTPS vs. HTTP
   * @return ESB_SUCCESS if the request contained enough information to
   * determine the peer address, another error code otherwise
   */
  ESB::Error parsePeerAddress(char *hostname, int size, ESB::UInt16 *port, bool *isSecure) const;

 private:
  unsigned const char *_method;
  HttpRequestUri _requestUri;

  ESB_DEFAULT_FUNCS(HttpRequest);
};

}  // namespace ES

#endif
