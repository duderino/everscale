#ifndef ES_HTTP_RESPONSE_H
#define ES_HTTP_RESPONSE_H

#ifndef ES_HTTP_MESSAGE_H
#include <ESHttpMessage.h>
#endif

namespace ES {

/**
 * A HTTP Response as defined in RFC 2616 and RFC 2396
 */
class HttpResponse : public HttpMessage {
 public:
  const char *DefaultReasonPhrase(int statusCode);

  HttpResponse();

  virtual ~HttpResponse();

  void reset();

  /**
   * Copy another response into this one, potentially filtering out unwanted headers.
   *
   * @param other The other response
   * @param allocator The allocator to use for memory for the copies
   * @param filter An optional callback to filter out unwanted headers
   * @return ESB_SUCCESS if successful, ESB_INVALID_FIELD if the header filter callback returns ES_HTTP_HEADER_ERROR,
   * another error code otherwise.
   */
  ESB::Error copy(const HttpResponse *other, ESB::Allocator &allocator,
                  es_http_header_filter filter = DefaultHeaderFilter, void *context = NULL);

  inline void setStatusCode(int statusCode) { _statusCode = statusCode; }

  inline int statusCode() const { return _statusCode; }

  inline void setReasonPhrase(const unsigned char *reasonPhrase) { _reasonPhrase = reasonPhrase; }

  inline void setReasonPhrase(const char *reasonPhrase) { _reasonPhrase = (const unsigned char *)reasonPhrase; }

  inline const unsigned char *reasonPhrase() const { return _reasonPhrase; }

 private:
  int _statusCode;
  unsigned const char *_reasonPhrase;

  ESB_DEFAULT_FUNCS(HttpResponse);
};

}  // namespace ES

#endif
