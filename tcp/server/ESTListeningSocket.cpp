#ifndef EST_LISTENING_SOCKET_H
#include <ESTListeningSocket.h>
#endif

#ifndef EST_SERVER_SOCKET_H
#include <ESTServerSocket.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace EST {

ListeningSocket::ListeningSocket(ESB::ListeningTCPSocket *socket, ESB::Allocator *allocator,
                                 ESB::SocketMultiplexerDispatcher *dispatcher, ESB::CleanupHandler *thisCleanupHandler,
                                 ESB::CleanupHandler *socketCleanupHandler)
    : _socket(socket),
      _allocator(allocator ? allocator : ESB::SystemAllocator::GetInstance()),
      _dispatcher(dispatcher),
      _thisCleanupHandler(thisCleanupHandler),
      _socketCleanupHandler(socketCleanupHandler) {}

ListeningSocket::~ListeningSocket() { ESB_LOG_DEBUG("ListeningSocket destroyed"); }

bool ListeningSocket::wantAccept() { return true; }

bool ListeningSocket::wantConnect() { return false; }

bool ListeningSocket::wantRead() { return false; }

bool ListeningSocket::wantWrite() { return false; }

bool ListeningSocket::isIdle() { return false; }

bool ListeningSocket::handleAcceptEvent(ESB::SocketMultiplexer &multiplexer) {
  assert(_socket);
  assert(_dispatcher);

  ESB::TCPSocket::AcceptData acceptData;

  assert(!_socket->isBlocking());

  ESB::Error error = ESB_SUCCESS;

  while (true) {
    error = _socket->accept(&acceptData);

    if (ESB_INTR != error) {
      break;
    }
  }

  if (ESB_AGAIN == error) {
    ESB_LOG_DEBUG("[listener:%d] not ready to accept - thundering herd", _socket->getSocketDescriptor());
    return true;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[listener:%d] error accepting new connection", _socket->getSocketDescriptor());
    return true;
  }

  ServerSocket *serverSocket = new (_allocator) ServerSocket(&acceptData, _socketCleanupHandler);

  if (!serverSocket) {
    ESB_LOG_WARNING("[listener:%d] Cannot allocate new server socket", _socket->getSocketDescriptor());
    ESB::TCPSocket::Close(acceptData._sockFd);
    return true;
  }

  error = _dispatcher->addMultiplexedSocket(serverSocket);

  if (ESB_SHUTDOWN == error) {
    ESB_LOG_WARNING(
        "[listener:%d] Dispatcher has been shutdown, closing newly "
        "accepted connection",
        _socket->getSocketDescriptor());
    _socketCleanupHandler->destroy(serverSocket);
    return true;
  }

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error,
                          "[listener:%d] Error adding newly accepted "
                          "connection to the dispatcher",
                          _socket->getSocketDescriptor());
    _socketCleanupHandler->destroy(serverSocket);
    return true;
  }

  if (ESB_INFO_LOGGABLE) {
    char dottedIP[ESB_IPV6_PRESENTATION_SIZE];
    acceptData._peerAddress.getIPAddress(dottedIP, sizeof(dottedIP));
    ESB_LOG_INFO("[listener:%d] Accepted new connection from %s:%d", _socket->getSocketDescriptor(), dottedIP,
                 acceptData._peerAddress.getPort());
  }

  return true;
}

bool ListeningSocket::handleConnectEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_WARNING("[listener:%d] Cannot handle connect events", _socket->getSocketDescriptor());
  return true;
}

bool ListeningSocket::handleReadableEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_WARNING("[listener:%d] Cannot handle readable events", _socket->getSocketDescriptor());
  return true;
}

bool ListeningSocket::handleWritableEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_WARNING("[listener:%d] Cannot handle writable events", _socket->getSocketDescriptor());
  return true;
}

bool ListeningSocket::handleErrorEvent(ESB::Error error, ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_ERROR_ERRNO(error, "[listener:%d] Error on socket", _socket->getSocketDescriptor());
  return true;
}

bool ListeningSocket::handleEndOfFileEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_WARNING("[listener:%d] Cannot handle eof events", _socket->getSocketDescriptor());
  return true;
}

bool ListeningSocket::handleIdleEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_WARNING("[listener:%d] Cannot handle idle events", _socket->getSocketDescriptor());
  return true;
}

bool ListeningSocket::handleRemoveEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_NOTICE("[listener:%d] Removed from multiplexer", _socket->getSocketDescriptor());
  return true;  // call cleanup handler on us after this returns
}

SOCKET ListeningSocket::getSocketDescriptor() const {
  return _socket ? _socket->getSocketDescriptor() : INVALID_SOCKET;
}

ESB::CleanupHandler *ListeningSocket::getCleanupHandler() { return _thisCleanupHandler; }

const char *ListeningSocket::getName() const { return "ListeningSocket"; }

}  // namespace EST
