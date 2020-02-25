#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

namespace ES {

HttpServerTransaction::HttpServerTransaction()
    : HttpTransaction(0),
      _parser(getWorkingBuffer(), &_allocator),
      _formatter() {}

HttpServerTransaction::~HttpServerTransaction() {}

void HttpServerTransaction::reset() {
  HttpTransaction::reset();
  _parser.reset();
  _formatter.reset();
}

}  // namespace ES
