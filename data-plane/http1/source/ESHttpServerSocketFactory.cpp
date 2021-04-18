#ifndef ES_HTTP_SERVER_SOCKET_FACTORY_H
#include <ESHttpServerSocketFactory.h>
#endif

#ifndef ESB_SERVER_TLS_SOCKET_H
#include <ESBServerTLSSocket.h>
#endif

#ifndef ESB_CLEAR_SOCKET_H
#include <ESBClearSocket.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

namespace ES {

HttpServerSocketFactory::HttpServerSocketFactory(HttpMultiplexerExtended &multiplexer, HttpServerHandler &handler,
                                                 HttpServerCounters &counters, ESB::ServerTLSContextIndex &contextIndex,
                                                 ESB::Allocator &allocator)
    : _contextIndex(contextIndex),
      _multiplexer(multiplexer),
      _handler(handler),
      _counters(counters),
      _allocator(allocator),
      _deconstructedHTTPSockets(),
      _deconstructedClearSockets(),
      _deconstructedTLSSockets(),
      _cleanupHandler(*this) {}

HttpServerSocketFactory::~HttpServerSocketFactory() {
  for (ESB::EmbeddedListElement *e = _deconstructedHTTPSockets.removeFirst(); e;
       e = _deconstructedHTTPSockets.removeFirst()) {
    _allocator.deallocate(e);
  }
  for (ESB::EmbeddedListElement *e = _deconstructedClearSockets.removeFirst(); e;
       e = _deconstructedClearSockets.removeFirst()) {
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
    case ESB::SocketAddress::TLS: {
      ESB::EmbeddedListElement *memory = _deconstructedTLSSockets.removeLast();
      socket = memory ? new (memory) ESB::ServerTLSSocket(state, _multiplexer.multiplexer().name(), _contextIndex)
                      : new (_allocator) ESB::ServerTLSSocket(state, _multiplexer.multiplexer().name(), _contextIndex);
      if (!socket) {
        error = ESB_OUT_OF_MEMORY;
      }
    } break;
    case ESB::SocketAddress::TCP: {
      ESB::EmbeddedListElement *memory = _deconstructedClearSockets.removeLast();
      socket = memory ? new (memory) ESB::ClearSocket(state, _multiplexer.multiplexer().name())
                      : new (_allocator) ESB::ClearSocket(state, _multiplexer.multiplexer().name());
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

  ESB::EmbeddedListElement *memory = _deconstructedHTTPSockets.removeFirst();
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

  return serverSocket;
}

void HttpServerSocketFactory::release(HttpServerSocket *serverSocket) {
  if (!serverSocket) {
    return;
  }

  releaseSocket(serverSocket->socket());
  serverSocket->~HttpServerSocket();
  _deconstructedHTTPSockets.addLast(serverSocket);
}

void HttpServerSocketFactory::releaseSocket(ESB::ConnectedSocket *socket) {
  if (!socket) {
    return;
  }

  ESB::SocketAddress::TransportType type = socket->peerAddress().type();
  socket->~ConnectedSocket();

  switch (type) {
    case ESB::SocketAddress::TLS:
      _deconstructedTLSSockets.addLast(socket);
      break;
    case ESB::SocketAddress::TCP:
      _deconstructedClearSockets.addLast(socket);
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
