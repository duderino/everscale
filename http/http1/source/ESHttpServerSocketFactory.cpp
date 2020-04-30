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

HttpServerSocketFactory::HttpServerSocketFactory(
    HttpMultiplexerExtended &multiplexer, HttpServerHandler &handler,
    HttpServerCounters &counters, ESB::Allocator &allocator)
    : _multiplexer(multiplexer),
      _handler(handler),
      _counters(counters),
      _allocator(allocator),
      _sockets(),
      _cleanupHandler(*this) {}

HttpServerSocketFactory::~HttpServerSocketFactory() {
  while (true) {
    HttpServerSocket *socket = (HttpServerSocket *)_sockets.removeFirst();
    if (!socket) {
      break;
    }
    socket->~HttpServerSocket();
    _allocator.deallocate(socket);
  }
}

HttpServerSocket *HttpServerSocketFactory::create(
    ESB::TCPSocket::State &state) {
  HttpServerSocket *socket = (HttpServerSocket *)_sockets.removeFirst();

  if (!socket) {
    socket = new (_allocator)
        HttpServerSocket(_handler, _multiplexer, _counters, _cleanupHandler);
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