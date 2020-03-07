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

ListeningSocket::ListeningSocket(ESB::ListeningTCPSocket *socket,
                                 ESB::Allocator *allocator,
                                 ESB::SocketMultiplexerDispatcher *dispatcher,
                                 ESB::Logger *logger,
                                 ESB::CleanupHandler *thisCleanupHandler,
                                 ESB::CleanupHandler *socketCleanupHandler)
    : _socket(socket),
      _allocator(allocator ? allocator : ESB::SystemAllocator::GetInstance()),
      _dispatcher(dispatcher),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _thisCleanupHandler(thisCleanupHandler),
      _socketCleanupHandler(socketCleanupHandler) {}

ListeningSocket::~ListeningSocket() {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "ListeningSocket destroyed");
  }
}

bool ListeningSocket::wantAccept() { return true; }

bool ListeningSocket::wantConnect() { return false; }

bool ListeningSocket::wantRead() { return false; }

bool ListeningSocket::wantWrite() { return false; }

bool ListeningSocket::isIdle() { return false; }

bool ListeningSocket::handleAcceptEvent(ESB::SharedInt *isRunning,
                                        ESB::Logger *logger) {
  assert(_socket);
  assert(_dispatcher);

  ESB::TCPSocket::AcceptData acceptData;

  assert(false == _socket->isBlocking());

  ESB::Error error = ESB_SUCCESS;

  while (true) {
    error = _socket->accept(&acceptData);

    if (ESB_INTR != error) {
      break;
    }
  }

  if (ESB_AGAIN == error) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[listener:%d] not ready to accept - thundering herd",
                   _socket->getSocketDescriptor());
    }

    return true;
  }

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Err)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                   "[listener:%d] error accepting new connection: %s",
                   _socket->getSocketDescriptor(), buffer);
    }

    return true;
  }

  ServerSocket *serverSocket = new (_allocator)
      ServerSocket(&acceptData, _socketCleanupHandler, _logger);

  if (!serverSocket) {
    if (_logger->isLoggable(ESB::Logger::Critical)) {
      _logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                   "[listener:%d] Cannot allocate new server socket",
                   _socket->getSocketDescriptor());
    }

    ESB::TCPSocket::Close(acceptData._sockFd);

    return true;
  }

  error = _dispatcher->addMultiplexedSocket(serverSocket);

  if (ESB_SHUTDOWN == error) {
    if (_logger->isLoggable(ESB::Logger::Warning)) {
      _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                   "[listener:%d] Dispatcher has been shutdown, closing newly "
                   "accepted connection",
                   _socket->getSocketDescriptor());
    }

    _socketCleanupHandler->destroy(serverSocket);

    return true;
  }

  if (ESB_SUCCESS != error) {
    if (_logger->isLoggable(ESB::Logger::Err)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                   "[listener:%d] Error adding newly accepted connection to "
                   "the dispatcher: %s",
                   _socket->getSocketDescriptor(), buffer);
    }

    _socketCleanupHandler->destroy(serverSocket);

    return true;
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char buffer[16];

    acceptData._peerAddress.getIPAddress(buffer, sizeof(buffer));

    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Accepted new connection from %s",
                 _socket->getSocketDescriptor(), buffer);
  }

  return true;
}

bool ListeningSocket::handleConnectEvent(ESB::SharedInt *isRunning,
                                         ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle connect events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool ListeningSocket::handleReadableEvent(ESB::SharedInt *isRunning,
                                          ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle readable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool ListeningSocket::handleWritableEvent(ESB::SharedInt *isRunning,
                                          ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle writable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool ListeningSocket::handleErrorEvent(ESB::Error errorCode,
                                       ESB::SharedInt *isRunning,
                                       ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    char buffer[100];

    ESB::DescribeError(errorCode, buffer, sizeof(buffer));

    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Error on socket: %s",
                 _socket->getSocketDescriptor(), buffer);
  }

  return true;
}

bool ListeningSocket::handleEndOfFileEvent(ESB::SharedInt *isRunning,
                                           ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle eof events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool ListeningSocket::handleIdleEvent(ESB::SharedInt *isRunning,
                                      ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle idle events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool ListeningSocket::handleRemoveEvent(ESB::SharedInt *flag,
                                        ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Notice)) {
    _logger->log(ESB::Logger::Notice, __FILE__, __LINE__,
                 "[listener:%d] Removed from multiplexer",
                 _socket->getSocketDescriptor());
  }

  return true;  // call cleanup handler on us after this returns
}

SOCKET ListeningSocket::getSocketDescriptor() const {
  return _socket ? _socket->getSocketDescriptor() : INVALID_SOCKET;
}

ESB::CleanupHandler *ListeningSocket::getCleanupHandler() {
  return _thisCleanupHandler;
}

const char *ListeningSocket::getName() const { return "ListeningSocket"; }

bool ListeningSocket::run(ESB::SharedInt *isRunning) {
  return false;  // todo - log
}

}  // namespace EST
