#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

namespace ES {

HttpClientTransaction::HttpClientTransaction(
    ESB::CleanupHandler &cleanupHandler)
    : HttpTransaction(cleanupHandler),
      _parser(getWorkingBuffer(), _allocator),
      _formatter() {}

HttpClientTransaction::HttpClientTransaction(
    ESB::SocketAddress *peerAddress, ESB::CleanupHandler &cleanupHandler)
    : HttpTransaction(peerAddress, cleanupHandler),
      _parser(getWorkingBuffer(), _allocator),
      _formatter() {}

HttpClientTransaction::~HttpClientTransaction() {}

void HttpClientTransaction::reset() {
  HttpTransaction::reset();

  _parser.reset();
  _formatter.reset();
}

}  // namespace ES
