#ifndef ES_HTTP_LISTENING_SOCKET_H
#include <ESHttpListeningSocket.h>
#endif

#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ES {

// TODO add performance counters

HttpListeningSocket::HttpListeningSocket(HttpMultiplexerExtended &stack, HttpServerHandler &handler,
                                         ESB::CleanupHandler &cleanupHandler)
    : _socket(stack.multiplexer().name()),
      _multiplexer(stack),
      _handler(handler),
      _cleanupHandler(cleanupHandler),
      _dead(false) {}

HttpListeningSocket::~HttpListeningSocket() {}

ESB::Error HttpListeningSocket::initialize(ESB::ListeningSocket &socket) { return _socket.duplicate(socket); }

bool HttpListeningSocket::wantAccept() {
  // todo return false if at max sockets...
  return true;
}

bool HttpListeningSocket::wantConnect() { return false; }

bool HttpListeningSocket::wantRead() { return false; }

bool HttpListeningSocket::wantWrite() { return false; }

ESB::Error HttpListeningSocket::handleAccept() {
  assert(!_socket.isBlocking());

  ESB::Socket::State state;
  ESB::Error error = _socket.accept(&state);

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] not ready to accept - no sockets", _socket.name());
    return ESB_SUCCESS;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error accepting new connection", _socket.name());
    return error;
  }

  _multiplexer.serverCounters().getTotalConnections()->inc();

  error = _handler.acceptConnection(_multiplexer, &state.peerAddress());

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] Handler rejected connection", _socket.name());
    ESB::Socket::Close(state.socketDescriptor());
    return ESB_AGAIN;  // keep calling accept until the OS returns EAGAIN
  }

  error = _multiplexer.addServerSocket(state);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add accepted connection to multiplexer", _socket.name());
    return ESB_AGAIN;  // keep calling accept until the OS returns EAGAIN
  }

  if (ESB_INFO_LOGGABLE) {
    char buffer[ESB_ADDRESS_PORT_SIZE + 1 + ESB_MAX_UINT32_STRING_LENGTH];
    state.peerAddress().logAddress(buffer, sizeof(buffer), state.socketDescriptor());
    ESB_LOG_INFO("[%s] accepted new connection [%s]", _socket.name(), buffer);
  }

  return ESB_AGAIN;  // keep calling accept until the OS returns EAGAIN
}

ESB::Error HttpListeningSocket::handleConnect() {
  ESB_LOG_ERROR("[%s] Cannot handle connect events", _socket.name());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpListeningSocket::handleReadable() {
  ESB_LOG_ERROR("[%s] Cannot handle readable events", _socket.name());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

ESB::Error HttpListeningSocket::handleWritable() {
  ESB_LOG_ERROR("[%s] Cannot handle writable events", _socket.name());
  return ESB_INVALID_STATE;  // remove from multiplexer
}

void HttpListeningSocket::handleError(ESB::Error error) {
  ESB_LOG_ERROR_ERRNO(error, "[%s] listening socket error", _socket.name());
}

void HttpListeningSocket::handleRemoteClose() { ESB_LOG_ERROR("[%s] Cannot handle eof events", _socket.name()); }

void HttpListeningSocket::handleIdle() { ESB_LOG_ERROR("[%s] Cannot handle idle events", _socket.name()); }

void HttpListeningSocket::handleRemove() { ESB_LOG_INFO("[%s] Removed from multiplexer", _socket.name()); }

SOCKET HttpListeningSocket::socketDescriptor() const { return _socket.socketDescriptor(); }

ESB::CleanupHandler *HttpListeningSocket::cleanupHandler() { return &_cleanupHandler; }

const char *HttpListeningSocket::name() const { return _socket.name(); }

const void *HttpListeningSocket::key() const { return _socket.key(); }

bool HttpListeningSocket::permanent() { return true; }

void HttpListeningSocket::markDead() { _dead = true; }

bool HttpListeningSocket::dead() const { return _dead; }

}  // namespace ES
