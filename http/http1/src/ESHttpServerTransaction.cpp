#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

namespace ES {

HttpServerTransaction::HttpServerTransaction()
    : HttpTransaction(0),
      _parseBuffer(_storage, sizeof(_storage)),
      _parser(&_parseBuffer, _allocator),
      _formatter() {}

HttpServerTransaction::~HttpServerTransaction() {}

void HttpServerTransaction::reset() {
  HttpTransaction::reset();
  _parser.reset();
  _formatter.reset();
  _parseBuffer.clear();
}

}  // namespace ES
