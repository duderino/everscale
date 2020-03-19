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

HttpListeningSocket::HttpListeningSocket(
    HttpServerHandler *handler, ESB::ListeningTCPSocket *socket,
    ESB::SocketMultiplexerDispatcher *dispatcher,
    HttpServerSocketFactory *factory, ESB::CleanupHandler *thisCleanupHandler,
    HttpServerCounters *counters)
    : _handler(handler),
      _socket(socket),
      _dispatcher(dispatcher),
      _thisCleanupHandler(thisCleanupHandler),
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

bool HttpListeningSocket::handleAcceptEvent(
    ESB::SocketMultiplexer &multiplexer) {
  assert(_socket);
  assert(!_socket->isBlocking());
  assert(_dispatcher);

  ESB::TCPSocket::AcceptData acceptData;
  ESB::Error error = ESB_SUCCESS;

  for (int i = 0; i < 10; ++i) {
    error = _socket->accept(&acceptData);
    if (ESB_INTR != error) {
      break;
    }
  }

  if (ESB_INTR == error) {
    ESB_LOG_DEBUG("listener:%d not ready to accept - too many interrupts",
                  _socket->getSocketDescriptor());
    return true;
  }

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("listener:%d not ready to accept - thundering herd",
                  _socket->getSocketDescriptor());
    return true;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_INFO_ERRNO(error, "listener:%d error accepting new connection",
                       _socket->getSocketDescriptor());
    return true;
  }

  _counters->getTotalConnections()->inc();

  HttpServerHandler::Result result =
      _handler->acceptConnection(&acceptData._peerAddress);

  if (HttpServerHandler::ES_HTTP_SERVER_HANDLER_CONTINUE != result) {
    ESB_LOG_DEBUG("listener:%d Handler rejected connection",
                  _socket->getSocketDescriptor());
    ESB::TCPSocket::Close(acceptData._sockFd);
    return true;
  }

  HttpServerSocket *serverSocket = _factory->create(_handler, &acceptData);

  if (!serverSocket) {
    ESB_LOG_CRITICAL("listener:%d cannot allocate new server socket",
                     _socket->getSocketDescriptor());
    ESB::TCPSocket::Close(acceptData._sockFd);
    return true;
  }

  error = _dispatcher->addMultiplexedSocket(serverSocket);

  if (ESB_SHUTDOWN == error) {
    ESB_LOG_WARNING(
        "listener:%d Dispatcher has been shutdown, closing newly "
        "accepted connection",
        _socket->getSocketDescriptor());
    _factory->release(serverSocket);
    return true;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error,
                        "listener:%d cannot add accepted connection to "
                        "dispatcher",
                        _socket->getSocketDescriptor());
    _factory->release(serverSocket);
    return true;
  }

  if (ESB_INFO_LOGGABLE) {
    char buffer[ESB_IPV6_PRESENTATION_SIZE];
    acceptData._peerAddress.getIPAddress(buffer, sizeof(buffer));
    ESB_LOG_INFO("listener:%d accepted new connection from %s",
                 _socket->getSocketDescriptor(), buffer);
  }

  return true;
}

bool HttpListeningSocket::handleConnectEvent(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle connect events",
                _socket->getSocketDescriptor());
  return true;
}

bool HttpListeningSocket::handleReadableEvent(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle readable events",
                _socket->getSocketDescriptor());
  return true;
}

bool HttpListeningSocket::handleWritableEvent(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle writable events",
                _socket->getSocketDescriptor());
  return true;
}

bool HttpListeningSocket::handleErrorEvent(
    ESB::Error error, ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR_ERRNO(error, "listener:%d listening socket error",
                      _socket->getSocketDescriptor());
  return true;
}

bool HttpListeningSocket::handleEndOfFileEvent(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle eof events",
                _socket->getSocketDescriptor());
  return true;
}

bool HttpListeningSocket::handleIdleEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR("listener:%d Cannot handle idle events",
                _socket->getSocketDescriptor());
  return true;
}

bool HttpListeningSocket::handleRemoveEvent(
    ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_INFO("listener:%d Removed from multiplexer",
               _socket->getSocketDescriptor());
  return true;  // call cleanup handler on us after this returns
}

SOCKET HttpListeningSocket::getSocketDescriptor() const {
  return _socket ? _socket->getSocketDescriptor() : INVALID_SOCKET;
}

ESB::CleanupHandler *HttpListeningSocket::getCleanupHandler() {
  return _thisCleanupHandler;
}

const char *HttpListeningSocket::getName() const {
  return "HttpListeningSocket";
}

}  // namespace ES
