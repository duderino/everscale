#ifndef ES_HTTP_MESSAGE_H
#define ES_HTTP_MESSAGE_H

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ES_HTTP_HEADER_H
#include <ESHttpHeader.h>
#endif

namespace ES {

#define ES_HTTP_MESSAGE_HAS_BODY (1 << 0)
#define ES_HTTP_MESSAGE_REUSE_CONNECTION (1 << 1)
#define ES_HTTP_MESSAGE_SEND_100_CONTINUE (1 << 2)

/**
 * A HTTP message as defined in RFC 2616 and RFC 2396
 */
class HttpMessage {
 public:
  HttpMessage();

  virtual ~HttpMessage();

  inline ESB::EmbeddedList *getHeaders() { return &_headers; }

  inline const ESB::EmbeddedList *getHeaders() const { return &_headers; }

  const HttpHeader *getHeader(unsigned const char *fieldName) const;

  ESB::Error addHeader(unsigned const char *fieldName,
                       unsigned const char *fieldValue,
                       ESB::Allocator *allocator);

  ESB::Error addHeader(ESB::Allocator *allocator,
                       unsigned const char *fieldName,
                       unsigned const char *fieldValueFormat, ...);

  /**
   * Get the HTTP version.
   *
   * @return The HTTP version.  110 is HTTP/1.1, 100 is HTTP/1.0, etc.
   */
  inline int getHttpVersion() const { return _version; }

  /**
   * Set the HTTP version.
   *
   * @param vesion The HTTP version.  110 is HTTP/1.1, 100 is HTTP/1.0, etc.
   */
  inline void setHttpVersion(int version) { _version = version; }

  inline void reset() {
    _version = 110;
    _headers.clear(false);  // don't call cleanup handlers
  }

  inline void setHasBody(bool hasBody) {
    if (hasBody) {
      _flags |= ES_HTTP_MESSAGE_HAS_BODY;
    } else {
      _flags &= ~ES_HTTP_MESSAGE_HAS_BODY;
    }
  }

  inline bool hasBody() const { return _flags & ES_HTTP_MESSAGE_HAS_BODY; }

  inline void setReuseConnection(bool reuseConnection) {
    if (reuseConnection) {
      _flags |= ES_HTTP_MESSAGE_REUSE_CONNECTION;
    } else {
      _flags &= ~ES_HTTP_MESSAGE_REUSE_CONNECTION;
    }
  }

  inline bool getReuseConnection() const {
    return _flags & ES_HTTP_MESSAGE_REUSE_CONNECTION;
  }

  inline void setSend100Continue(bool send100Continue) {
    if (send100Continue) {
      _flags |= ES_HTTP_MESSAGE_SEND_100_CONTINUE;
    } else {
      _flags &= ~ES_HTTP_MESSAGE_SEND_100_CONTINUE;
    }
  }

  inline bool getSend100Continue() {
    return _flags & ES_HTTP_MESSAGE_SEND_100_CONTINUE;
  }

 private:
  // Disabled
  HttpMessage(const HttpMessage &);
  void operator=(const HttpMessage &);

  int _flags;
  int _version;  // 110 is 1.1, 100 is 1.0, etc.
  ESB::EmbeddedList _headers;
};

}  // namespace ES

#endif