#ifndef ES_HTTP_SERVER_TRANSACTION_H
#include <ESHttpServerTransaction.h>
#endif

namespace ES {

HttpServerTransaction::HttpServerTransaction(ESB::CleanupHandler &cleanupHandler)
    : HttpTransaction(cleanupHandler), _parser(parseBuffer(), _allocator), _formatter() {}

HttpServerTransaction::~HttpServerTransaction() {}

void HttpServerTransaction::reset() {
  HttpTransaction::reset();
  _parser.reset();
  _formatter.reset();
}

}  // namespace ES
