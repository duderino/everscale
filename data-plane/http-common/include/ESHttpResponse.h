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
  HttpResponse();

  virtual ~HttpResponse();

  void reset();

  ESB::Error copy(const HttpResponse *other, ESB::Allocator &allocator);

  inline void setStatusCode(int statusCode) { _statusCode = statusCode; }

  inline int statusCode() const { return _statusCode; }

  inline void setReasonPhrase(const unsigned char *reasonPhrase) { _reasonPhrase = reasonPhrase; }

  inline void setReasonPhrase(const char *reasonPhrase) { _reasonPhrase = (const unsigned char *)reasonPhrase; }

  inline const unsigned char *reasonPhrase() const { return _reasonPhrase; }

 private:
  // Disabled
  HttpResponse(const HttpResponse &);
  void operator=(const HttpResponse &);

  int _statusCode;
  unsigned const char *_reasonPhrase;
  char _pad[8];
};

}  // namespace ES

#endif