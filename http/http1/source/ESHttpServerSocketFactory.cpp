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

HttpServerSocketFactory::HttpServerSocketFactory(HttpMultiplexerExtended &multiplexer, HttpServerHandler &handler,
                                                 HttpServerCounters &counters, ESB::Allocator &allocator)
    : _multiplexer(multiplexer),
      _handler(handler),
      _counters(counters),
      _allocator(allocator),
      _deconstructedServerSockets(),
      _deconstructedTCPSockets(),
      _deconstructedTLSSockets(),
      _cleanupHandler(*this) {}

HttpServerSocketFactory::~HttpServerSocketFactory() {
  for (ESB::EmbeddedListElement *e = _deconstructedServerSockets.removeFirst(); e;
       e = _deconstructedServerSockets.removeFirst()) {
    _allocator.deallocate(e);
  }
  for (ESB::EmbeddedListElement *e = _deconstructedTCPSockets.removeFirst(); e;
       e = _deconstructedTCPSockets.removeFirst()) {
    _allocator.deallocate(e);
  }
  for (ESB::EmbeddedListElement *e = _deconstructedTLSSockets.removeFirst(); e;
       e = _deconstructedTLSSockets.removeFirst()) {
    _allocator.deallocate(e);
  }
}

HttpServerSocket *HttpServerSocketFactory::create(ESB::Socket::State &state) {
  ESB::ConnectedSocket *socket = NULL;
  ESB::Error error = ESB_SUCCESS;

  switch (state.peerAddress().type()) {
    case ESB::SocketAddress::TLS:
      assert(!"support TLS");
    case ESB::SocketAddress::TCP: {
      ESB::EmbeddedListElement *memory = _deconstructedTCPSockets.removeLast();
      socket = memory
                   ? new (memory)
                         ESB::ConnectedSocket(_multiplexer.multiplexer().name(), "server", state.peerAddress(), false)
                   : new (_allocator)
                         ESB::ConnectedSocket(_multiplexer.multiplexer().name(), "server", state.peerAddress(), false);
      if (!socket) {
        error = ESB_OUT_OF_MEMORY;
      }
    } break;
    default:
      error = ESB_UNSUPPORTED_TRANSPORT;
  }

  if (ESB_SUCCESS != error) {
    if (ESB_ERROR_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      state.peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot accept from [%s:%u]", _multiplexer.multiplexer().name(),
                          presentationAddress, state.peerAddress().port());
    }
    return NULL;
  }

  ESB::EmbeddedListElement *memory = _deconstructedServerSockets.removeFirst();
  HttpServerSocket *serverSocket =
      memory ? new (memory) HttpServerSocket(socket, _handler, _multiplexer, _counters, _cleanupHandler)
             : new (_allocator) HttpServerSocket(socket, _handler, _multiplexer, _counters, _cleanupHandler);

  if (!serverSocket) {
    if (ESB_ERROR_LOGGABLE) {
      char presentationAddress[ESB_IPV6_PRESENTATION_SIZE];
      state.peerAddress().presentationAddress(presentationAddress, sizeof(presentationAddress));
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot accept from [%s:%u]", _multiplexer.multiplexer().name(),
                          presentationAddress, state.peerAddress().port());
    }
    releaseSocket(socket);
    return NULL;
  }

  socket->reset(state);

  return serverSocket;
}

void HttpServerSocketFactory::release(HttpServerSocket *serverSocket) {
  if (!serverSocket) {
    return;
  }

  releaseSocket(serverSocket->socket());
  serverSocket->~HttpServerSocket();
  _deconstructedServerSockets.addLast(serverSocket);
}

void HttpServerSocketFactory::releaseSocket(ESB::ConnectedSocket *socket) {
  if (!socket) {
    return;
  }

  socket->~ConnectedSocket();

  switch (socket->peerAddress().type()) {
    case ESB::SocketAddress::TLS:
      _deconstructedTLSSockets.addLast(socket);
      break;
    case ESB::SocketAddress::TCP:
      _deconstructedTCPSockets.addLast(socket);
      break;
    default:
      _allocator.deallocate(socket);
  }
}

HttpServerSocketFactory::CleanupHandler::CleanupHandler(HttpServerSocketFactory &factory)
    : ESB::CleanupHandler(), _factory(factory) {}

HttpServerSocketFactory::CleanupHandler::~CleanupHandler() {}

void HttpServerSocketFactory::CleanupHandler::destroy(ESB::Object *object) {
  _factory.release((HttpServerSocket *)object);
}

}  // namespace ES
