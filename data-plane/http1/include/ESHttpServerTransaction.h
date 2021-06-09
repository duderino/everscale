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

  inline HttpRequestParser *getParser() { return &_parser; }

  inline const HttpRequestParser *getParser() const { return &_parser; }

  inline HttpResponseFormatter *getFormatter() { return &_formatter; }

  inline const HttpResponseFormatter *getFormatter() const { return &_formatter; }

 private:
  HttpRequestParser _parser;
  HttpResponseFormatter _formatter;

  ESB_DEFAULT_FUNCS(HttpServerTransaction);
};

}  // namespace ES

#endif
