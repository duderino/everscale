#ifndef ES_HTTP_LISTENING_SOCKET_H
#include <ESHttpListeningSocket.h>
#endif

#ifndef ES_HTTP_SERVER_SOCKET_H
#include <ESHttpServerSocket.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

namespace ES {

// TODO add performance counters

HttpListeningSocket::HttpListeningSocket(
    HttpServerHandler *handler, ESB::ListeningTCPSocket *socket,
    ESB::SocketMultiplexerDispatcher *dispatcher,
    HttpServerSocketFactory *factory, ESB::Logger *logger,
    ESB::CleanupHandler *thisCleanupHandler, HttpServerCounters *counters)
    : _handler(handler),
      _socket(socket),
      _dispatcher(dispatcher),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _thisCleanupHandler(thisCleanupHandler),
      _factory(factory),
      _counters(counters) {}

HttpListeningSocket::~HttpListeningSocket() {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "HttpListeningSocket destroyed");
  }
}

bool HttpListeningSocket::wantAccept() {
  // todo return false if at max sockets...

  return true;
}

bool HttpListeningSocket::wantConnect() { return false; }

bool HttpListeningSocket::wantRead() { return false; }

bool HttpListeningSocket::wantWrite() { return false; }

bool HttpListeningSocket::isIdle() { return false; }

bool HttpListeningSocket::handleAcceptEvent(ESB::SharedInt *isRunning,
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

  _counters->getTotalConnections()->inc();

  HttpServerHandler::Result result =
      _handler->acceptConnection(&acceptData._peerAddress);

  if (HttpServerHandler::ES_HTTP_SERVER_HANDLER_CONTINUE != result) {
    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[listener:%d] Handler rejected connection",
                   _socket->getSocketDescriptor());
    }
    ESB::TCPSocket::Close(acceptData._sockFd);
    return true;
  }

  HttpServerSocket *serverSocket = _factory->create(_handler, &acceptData);

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
    _factory->release(serverSocket);
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
    _factory->release(serverSocket);
    return true;
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char buffer[16];
    acceptData._peerAddress.getIPAddress(buffer, sizeof(buffer));
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[listener:%d] Accepted new connection from %s",
                 _socket->getSocketDescriptor(), buffer);
  }

  return true;
}

bool HttpListeningSocket::handleConnectEvent(ESB::SharedInt *isRunning,
                                             ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle connect events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool HttpListeningSocket::handleReadableEvent(ESB::SharedInt *isRunning,
                                              ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle readable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool HttpListeningSocket::handleWritableEvent(ESB::SharedInt *isRunning,
                                              ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle writable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool HttpListeningSocket::handleErrorEvent(ESB::Error errorCode,
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

bool HttpListeningSocket::handleEndOfFileEvent(ESB::SharedInt *isRunning,
                                               ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle eof events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool HttpListeningSocket::handleIdleEvent(ESB::SharedInt *isRunning,
                                          ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Err)) {
    _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle idle events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool HttpListeningSocket::handleRemoveEvent(ESB::SharedInt *flag,
                                            ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Notice)) {
    _logger->log(ESB::Logger::Notice, __FILE__, __LINE__,
                 "[listener:%d] Removed from multiplexer",
                 _socket->getSocketDescriptor());
  }

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

bool HttpListeningSocket::run(ESB::SharedInt *isRunning) {
  return false;  // todo - log
}

}  // namespace ES
