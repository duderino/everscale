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

  inline ESB::EmbeddedList &headers() { return _headers; }

  inline const ESB::EmbeddedList &headers() const { return _headers; }

  const HttpHeader *findHeader(const char *fieldName) const;

  inline ESB::Error addHeader(const HttpHeader *header, ESB::Allocator &allocator) {
    return addHeader((const char *)header->fieldName(), (const char *)header->fieldValue(), allocator);
  }

  ESB::Error addHeader(const char *fieldName, const char *fieldValue, ESB::Allocator &allocator);

  ESB::Error addHeader(ESB::Allocator &allocator, const char *fieldName, const char *fieldValueFormat, ...)
      __attribute__((format(printf, 4, 5)));

  typedef enum {
    ES_HTTP_HEADER_COPY = 0,          /**< Copy the header */
    ES_HTTP_HEADER_SKIP = 1,          /**< Skip the header */
    ES_HTTP_HEADER_COPY_AND_STOP = 2, /**< Copy the header and stop processing more headers */
    ES_HTTP_HEADER_SKIP_AND_STOP = 3, /**< Skip the header and stop processing more headers */
    ES_HTTP_HEADER_ERROR = 4          /**< Send a 400 bad response */
  } HeaderCopyResult;

  typedef HeaderCopyResult (*HeaderCopyFilter)(const unsigned char *fieldName, const unsigned char *fieldValue,
                                               void *context);
  static HeaderCopyResult HeaderCopyAll(const unsigned char *fieldName, const unsigned char *fieldValue, void *context);

  /**
   * Copy headers from another message into this one, potentially filtering out unwanted headers.
   *
   * @param headers The headers to copy
   * @param allocator The allocator to use for memory for the copies
   * @param filter An optional callback to filter out unwanted headers
   * @return ESB_SUCCESS if successful, ESB_INVALID_FIELD if the header filter callback returns ES_HTTP_HEADER_ERROR,
   * another error code otherwise.
   */
  ESB::Error copyHeaders(const ESB::EmbeddedList &headers, ESB::Allocator &allocator,
                         HeaderCopyFilter filter = HeaderCopyAll, void *context = NULL);

  /**
   * Get the HTTP version.
   *
   * @return The HTTP version.  110 is HTTP/1.1, 100 is HTTP/1.0, etc.
   */
  inline int httpVersion() const { return _version; }

  /**
   * Set the HTTP version.
   *
   * @param vesion The HTTP version.  110 is HTTP/1.1, 100 is HTTP/1.0, etc.
   */
  inline void setHttpVersion(int version) { _version = version; }

  inline void reset() {
    _version = 110;
    _headers.clear();
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

  inline bool reuseConnection() const { return _flags & ES_HTTP_MESSAGE_REUSE_CONNECTION; }

  inline void setSend100Continue(bool send100Continue) {
    if (send100Continue) {
      _flags |= ES_HTTP_MESSAGE_SEND_100_CONTINUE;
    } else {
      _flags &= ~ES_HTTP_MESSAGE_SEND_100_CONTINUE;
    }
  }

  inline bool send100Continue() { return _flags & ES_HTTP_MESSAGE_SEND_100_CONTINUE; }

 protected:
  inline int flags() const { return _flags; }

  inline void setFlags(int flags) { _flags = flags; }

 private:
  int _flags;
  int _version;  // 110 is 1.1, 100 is 1.0, etc.
  ESB::EmbeddedList _headers;

  ESB_DISABLE_AUTO_COPY(HttpMessage);
};

}  // namespace ES

#endif
