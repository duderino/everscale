#ifndef ES_HTTP_SERVER_TRANSACTION_H
#define ES_HTTP_SERVER_TRANSACTION_H

#ifndef ES_HTTP_TRANSACTION_H
#include <ESHttpTransaction.h>
#endif

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_REQUEST_PARSER_H
#include <ESHttpRequestParser.h>
#endif

#ifndef ES_HTTP_RESPONSE_FORMATTER_H
#include <ESHttpResponseFormatter.h>
#endif

namespace ES {

class HttpServerTransaction : public HttpTransaction {
 public:
  HttpServerTransaction();

  virtual ~HttpServerTransaction();

  virtual void reset();

  inline HttpRequestParser *getParser() { return &_parser; }

  inline HttpResponseFormatter *getFormatter() { return &_formatter; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 protected:
 private:
  // Disabled
  HttpServerTransaction(const HttpServerTransaction &transaction);
  void operator=(const HttpServerTransaction &transaction);

  ESB::Buffer _parseBuffer;          // 320 from base + 64 = 384
  HttpRequestParser _parser;         // 384 + 128 = 512
  HttpResponseFormatter _formatter;  // 512 + 64 = 576
  unsigned char _storage[2048 - 576];
};

}  // namespace ES

#endif
