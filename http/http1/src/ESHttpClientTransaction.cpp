#ifndef ES_HTTP_CLIENT_TRANSACTION_H
#include <ESHttpClientTransaction.h>
#endif

namespace ES {

HttpClientTransaction::HttpClientTransaction(
    HttpClientHandler *clientHandler, ESB::CleanupHandler *cleanupHandler)
    : HttpTransaction(cleanupHandler),
      _clientHandler(clientHandler),
      _parser(getWorkingBuffer(), &_allocator),
      _formatter() {}

HttpClientTransaction::HttpClientTransaction(
    HttpClientHandler *clientHandler, ESB::SocketAddress *peerAddress,
    ESB::CleanupHandler *cleanupHandler)
    : HttpTransaction(peerAddress, cleanupHandler),
      _clientHandler(clientHandler),
      _parser(getWorkingBuffer(), &_allocator),
      _formatter() {}

HttpClientTransaction::~HttpClientTransaction() {}

void HttpClientTransaction::reset() {
  HttpTransaction::reset();

  _parser.reset();
  _formatter.reset();
  _clientHandler = 0;
}

}  // namespace ES
