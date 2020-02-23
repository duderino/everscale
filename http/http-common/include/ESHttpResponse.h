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

  inline void setStatusCode(int statusCode) { _statusCode = statusCode; }

  inline int getStatusCode() const { return _statusCode; }

  inline void setReasonPhrase(const unsigned char *reasonPhrase) {
    _reasonPhrase = reasonPhrase;
  }

  inline const unsigned char *getReasonPhrase() const { return _reasonPhrase; }

 private:
  // Disabled
  HttpResponse(const HttpResponse &);
  void operator=(const HttpResponse &);

  int _statusCode;
  unsigned const char *_reasonPhrase;
};

}  // namespace ES

#endif