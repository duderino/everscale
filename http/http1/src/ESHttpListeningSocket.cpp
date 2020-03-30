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

HttpListeningSocket::HttpListeningSocket(HttpServerHandler &handler,
                                         ESB::ListeningTCPSocket &socket,
                                         HttpServerSocketFactory &factory,
                                         HttpServerCounters &counters)
    : _handler(handler),
      _socket(socket),
      _factory(factory),
      _counters(counters) {}

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
    ESB_LOG_DEBUG("listener:%d not ready to accept - too many interrupts",
                  _socket.socketDescriptor());
    return true;
  }

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("listener:%d not ready to accept - thundering herd",
                  _socket.socketDescriptor());
    return true;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "listener:%d error accepting new connection",
                       _socket.socketDescriptor());
    return true;
  }

  _counters.getTotalConnections()->inc();

  HttpServerHandler::Result result =
      _handler.acceptConnection(multiplexer, &state.peerAddress());

  if (HttpServerHandler::ES_HTTP_SERVER_HANDLER_CONTINUE != result) {
    ESB_LOG_DEBUG("listener:%d Handler rejected connection",
                  _socket.socketDescriptor());
    ESB::TCPSocket::Close(state.socketDescriptor());
    return true;
  }

  HttpServerSocket *serverSocket = _factory.create(state);

  if (!serverSocket) {
    ESB_LOG_CRITICAL("listener:%d cannot allocate new server socket",
                     _socket.socketDescriptor());
    ESB::TCPSocket::Close(state.socketDescriptor());
    return true;
  }

  error = multiplexer.addMultiplexedSocket(serverSocket);

  if (ESB_SHUTDOWN == error) {
    ESB_LOG_WARNING(
        "listener:%d Dispatcher has been shutdown, closing newly "
        "accepted connection",
        _socket.socketDescriptor());
    _factory.release(serverSocket);
    return true;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error,
                        "listener:%d cannot add accepted connection to "
                        "dispatcher",
                        _socket.socketDescriptor());
    _factory.release(serverSocket);
    return true;
  }

  if (ESB_INFO_LOGGABLE) {
    char buffer[ESB_IPV6_PRESENTATION_SIZE];
    state.peerAddress().presentationAddress(buffer, sizeof(buffer));
    ESB_LOG_INFO("listener:%d accepted new connection from %s",
                 _socket.socketDescriptor(), buffer);
  }

  return true;
}

bool HttpListeningSocket::handleConnect(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle connect events",
                _socket.socketDescriptor());
  return true;
}

bool HttpListeningSocket::handleReadable(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle readable events",
                _socket.socketDescriptor());
  return true;
}

bool HttpListeningSocket::handleWritable(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle writable events",
                _socket.socketDescriptor());
  return true;
}

bool HttpListeningSocket::handleError(ESB::Error error,
                                      ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR_ERRNO(error, "listener:%d listening socket error",
                      _socket.socketDescriptor());
  return true;
}

bool HttpListeningSocket::handleRemoteClose(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle eof events",
                _socket.socketDescriptor());
  return true;
}

bool HttpListeningSocket::handleIdle(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle idle events",
                _socket.socketDescriptor());
  return true;
}

bool HttpListeningSocket::handleRemove(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_INFO("listener:%d Removed from multiplexer",
               _socket.socketDescriptor());
  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpListeningSocket::socketDescriptor() const {
  return _socket.socketDescriptor();
}

ESB::CleanupHandler *HttpListeningSocket::cleanupHandler() { return NULL; }

const char *HttpListeningSocket::getName() const {
  return "HttpListeningSocket";
}

}  // namespace ES
