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

HttpListeningSocket::HttpListeningSocket(ESB::ListeningTCPSocket &socket,
                                         HttpServerStack &stack,
                                         HttpServerHandler &handler,
                                         ESB::CleanupHandler &cleanupHandler)
    : _socket(socket),
      _stack(stack),
      _handler(handler),
      _cleanupHandler(cleanupHandler) {}

HttpListeningSocket::~HttpListeningSocket() {
  ESB_LOG_DEBUG("HttpListeningSocket destroyed");
}

bool HttpListeningSocket::wantAccept() {
  // todo return false if at max sockets...

  return true;
}

bool HttpListeningSocket::wantConnect() { return false; }

bool HttpListeningSocket::wantRead() { return false; }

bool HttpListeningSocket::wantWrite() { return false; }

bool HttpListeningSocket::isIdle() { return false; }

bool HttpListeningSocket::handleAccept(ESB::SocketMultiplexer &multiplexer) {
  assert(!_socket.isBlocking());

  ESB::TCPSocket::State state;
  ESB::Error error = ESB_SUCCESS;

  for (int i = 0; i < 10; ++i) {
    error = _socket.accept(&state);
    if (ESB_INTR != error) {
      break;
    }
  }

  if (ESB_INTR == error) {
    ESB_LOG_DEBUG("[%s] not ready to accept - too many interrupts",
                  _socket.logAddress());
    return true;
  }

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[%s] not ready to accept - thundering herd",
                  _socket.logAddress());
    return true;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "[%s] error accepting new connection",
                       _socket.logAddress());
    return true;
  }

  _stack.counters().getTotalConnections()->inc();

  HttpServerHandler::Result result =
      _handler.acceptConnection(_stack, &state.peerAddress());

  if (HttpServerHandler::ES_HTTP_SERVER_HANDLER_CONTINUE != result) {
    ESB_LOG_DEBUG("[%s] Handler rejected connection", _socket.logAddress());
    ESB::TCPSocket::Close(state.socketDescriptor());
    return true;
  }

  error = _stack.addServerSocket(state);

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error,
                        "[%s] cannot add accepted connection to multiplexer",
                        _socket.logAddress());
    return true;
  }

  if (ESB_INFO_LOGGABLE) {
    char buffer[ESB_LOG_ADDRESS_SIZE];
    state.peerAddress().logAddress(buffer, sizeof(buffer),
                                   state.socketDescriptor());
    ESB_LOG_INFO("[%s] accepted new connection [%s]", _socket.logAddress(),
                 buffer);
  }

  return true;
}

bool HttpListeningSocket::handleConnect(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("[%s] Cannot handle connect events", _socket.logAddress());
  return true;
}

bool HttpListeningSocket::handleReadable(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("[%s] Cannot handle readable events", _socket.logAddress());
  return true;
}

bool HttpListeningSocket::handleWritable(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("[%s] Cannot handle writable events", _socket.logAddress());
  return true;
}

bool HttpListeningSocket::handleError(ESB::Error error,
                                      ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR_ERRNO(error, "[%s] listening socket error",
                      _socket.logAddress());
  return true;
}

bool HttpListeningSocket::handleRemoteClose(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("[%s] Cannot handle eof events", _socket.logAddress());
  return true;
}

bool HttpListeningSocket::handleIdle(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("[%s] Cannot handle idle events", _socket.logAddress());
  return true;
}

bool HttpListeningSocket::handleRemove(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_INFO("[%s] Removed from multiplexer", _socket.logAddress());
  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpListeningSocket::socketDescriptor() const {
  return _socket.socketDescriptor();
}

ESB::CleanupHandler *HttpListeningSocket::cleanupHandler() {
  return &_cleanupHandler;
}

const char *HttpListeningSocket::getName() const {
  return "HttpListeningSocket";
}

}  // namespace ES
