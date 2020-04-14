#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#define ES_HTTP_CLIENT_TRANSACTION_H

#ifndef ES_HTTP_TRANSACTION_H
#include <ESHttpTransaction.h>
#endif

#ifndef ES_HTTP_REQUEST_FORMATTER_H
#include <ESHttpRequestFormatter.h>
#endif

#ifndef ES_HTTP_RESPONSE_PARSER_H
#include <ESHttpResponseParser.h>
#endif

namespace ES {

class HttpClientTransaction : public HttpTransaction {
 public:
  HttpClientTransaction(ESB::CleanupHandler &cleanupHandler);

  HttpClientTransaction(ESB::SocketAddress *peerAddress,
                        ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpClientTransaction();

  virtual void reset();

  inline HttpResponseParser *getParser() { return &_parser; }

  inline const HttpResponseParser *getParser() const { return &_parser; }

  inline HttpRequestFormatter *getFormatter() { return &_formatter; }

  inline const HttpRequestFormatter *getFormatter() const {
    return &_formatter;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  HttpClientTransaction(const HttpClientTransaction &transaction);
  void operator=(const HttpClientTransaction &transaction);

  HttpResponseParser _parser;
  HttpRequestFormatter _formatter;
};

}  // namespace ES

#endif
