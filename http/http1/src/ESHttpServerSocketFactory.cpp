#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#include <ESHttpServerSocketFactory.h>
#endif

namespace ES {

HttpServerSocketFactory::HttpServerSocketFactory(HttpServerHandler &handler,
                                                 HttpServerCounters &counters,
                                                 ESB::Allocator &allocator)
    : _handler(handler),
      _counters(counters),
      _allocator(allocator),
      _sockets(),
      _cleanupHandler(*this) {}

HttpServerSocketFactory::~HttpServerSocketFactory() {
  HttpServerSocket *socket = (HttpServerSocket *)_sockets.removeFirst();

  while (socket) {
    socket->~HttpServerSocket();
    socket = (HttpServerSocket *)_sockets.removeFirst();
  }
}

HttpServerSocket *HttpServerSocketFactory::create(
    ESB::TCPSocket::State &state) {
  HttpServerSocket *socket = (HttpServerSocket *)_sockets.removeFirst();

  if (!socket) {
    socket = new (_allocator)
        HttpServerSocket(&_handler, &_cleanupHandler, &_counters);
    if (!socket) {
      return NULL;
    }
  }

  socket->reset(&_handler, state);
  return socket;
}

void HttpServerSocketFactory::release(HttpServerSocket *socket) {
  if (!socket) {
    return;
  }

  _sockets.addFirst(socket);
}

HttpServerSocketFactory::CleanupHandler::CleanupHandler(
    HttpServerSocketFactory &factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpServerSocketFactory::CleanupHandler::~CleanupHandler() {}

void HttpServerSocketFactory::CleanupHandler::destroy(ESB::Object *object) {
  _factory.release((HttpServerSocket *)object);
}

}  // namespace ES
