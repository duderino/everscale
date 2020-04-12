#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#include <ESHttpServerSocketFactory.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
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
  if (!_stack) {
    return NULL;
  }

  HttpServerSocket *socket = (HttpServerSocket *)_sockets.removeFirst();

  if (!socket) {
    socket = new (_allocator)
        HttpServerSocket(_handler, *_stack, _counters, _cleanupHandler);
    if (!socket) {
      return NULL;
    }
  }

  socket->reset(state);
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
