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
  HttpClientTransaction(ESB::CleanupHandler *cleanupHandler);

  HttpClientTransaction(ESB::SocketAddress *peerAddress,
                        ESB::CleanupHandler *cleanupHandler);

  virtual ~HttpClientTransaction();

  inline HttpResponseParser *getParser() { return &_parser; }

  inline HttpRequestFormatter *getFormatter() { return &_formatter; }

  virtual void reset();

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

  ESB::Buffer _parseBuffer;         // 320 from base + 64 = 384
  HttpResponseParser _parser;       // 384+64 = 448
  HttpRequestFormatter _formatter;  // 448+64 = 512
  unsigned char _storage[2048 - 512];
};

/** TODO replace with something more generic */
class HttpSeedTransactionHandler {
 public:
  virtual ESB::Error modifyTransaction(
      HttpClientTransaction *clientTransaction) = 0;
};

}  // namespace ES

#endif
