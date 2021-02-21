#ifndef ES_HTTP_SERVER_TRANSACTION_H
#define ES_HTTP_SERVER_TRANSACTION_H

#ifndef ES_HTTP_TRANSACTION_H
#include <ESHttpTransaction.h>
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
  HttpServerTransaction(ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpServerTransaction();

  virtual void reset();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { return allocator.allocate(size); }

  inline HttpRequestParser *getParser() { return &_parser; }

  inline const HttpRequestParser *getParser() const { return &_parser; }

  inline HttpResponseFormatter *getFormatter() { return &_formatter; }

  inline const HttpResponseFormatter *getFormatter() const { return &_formatter; }

 private:
  // Disabled
  HttpServerTransaction(const HttpServerTransaction &transaction);
  void operator=(const HttpServerTransaction &transaction);

  HttpRequestParser _parser;
  HttpResponseFormatter _formatter;
};

}  // namespace ES

#endif
