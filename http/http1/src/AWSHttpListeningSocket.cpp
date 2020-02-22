/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_LISTENING_SOCKET_H
#include <AWSHttpListeningSocket.h>
#endif

#ifndef AWS_HTTP_SERVER_SOCKET_H
#include <AWSHttpServerSocket.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

// TODO add performance counters

AWSHttpListeningSocket::AWSHttpListeningSocket(
    AWSHttpServerHandler *handler, ESFListeningTCPSocket *socket,
    ESFSocketMultiplexerDispatcher *dispatcher,
    AWSHttpServerSocketFactory *factory, ESFLogger *logger,
    ESFCleanupHandler *thisCleanupHandler, AWSHttpServerCounters *counters)
    : _handler(handler),
      _socket(socket),
      _dispatcher(dispatcher),
      _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _thisCleanupHandler(thisCleanupHandler),
      _factory(factory),
      _counters(counters) {}

AWSHttpListeningSocket::~AWSHttpListeningSocket() {
  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "AWSHttpListeningSocket destroyed");
  }
}

bool AWSHttpListeningSocket::wantAccept() {
  // todo return false if at max sockets...

  return true;
}

bool AWSHttpListeningSocket::wantConnect() { return false; }

bool AWSHttpListeningSocket::wantRead() { return false; }

bool AWSHttpListeningSocket::wantWrite() { return false; }

bool AWSHttpListeningSocket::isIdle() { return false; }

bool AWSHttpListeningSocket::handleAcceptEvent(ESFFlag *isRunning,
                                               ESFLogger *logger) {
  ESF_ASSERT(_socket);
  ESF_ASSERT(_dispatcher);

  ESFTCPSocket::AcceptData acceptData;

  ESF_ASSERT(false == _socket->isBlocking());

  ESFError error = ESF_SUCCESS;

  while (true) {
    error = _socket->accept(&acceptData);

    if (ESF_INTR != error) {
      break;
    }
  }

  if (ESF_AGAIN == error) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[listener:%d] not ready to accept - thundering herd",
                   _socket->getSocketDescriptor());
    }

    return true;
  }

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[listener:%d] error accepting new connection: %s",
                   _socket->getSocketDescriptor(), buffer);
    }

    return true;
  }

  _counters->getTotalConnections()->inc();

  AWSHttpServerHandler::Result result =
      _handler->acceptConnection(&acceptData._peerAddress);

  if (AWSHttpServerHandler::AWS_HTTP_SERVER_HANDLER_CONTINUE != result) {
    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[listener:%d] Handler rejected connection",
                   _socket->getSocketDescriptor());
    }

    ESFTCPSocket::Close(acceptData._sockFd);

    return true;
  }

  AWSHttpServerSocket *serverSocket = _factory->create(_handler, &acceptData);

  if (!serverSocket) {
    if (_logger->isLoggable(ESFLogger::Critical)) {
      _logger->log(ESFLogger::Critical, __FILE__, __LINE__,
                   "[listener:%d] Cannot allocate new server socket",
                   _socket->getSocketDescriptor());
    }

    ESFTCPSocket::Close(acceptData._sockFd);

    return true;
  }

  error = _dispatcher->addMultiplexedSocket(serverSocket);

  if (ESF_SHUTDOWN == error) {
    if (_logger->isLoggable(ESFLogger::Warning)) {
      _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                   "[listener:%d] Dispatcher has been shutdown, closing newly "
                   "accepted connection",
                   _socket->getSocketDescriptor());
    }

    _factory->release(serverSocket);

    return true;
  }

  if (ESF_SUCCESS != error) {
    if (_logger->isLoggable(ESFLogger::Error)) {
      char buffer[100];

      ESFDescribeError(error, buffer, sizeof(buffer));

      _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                   "[listener:%d] Error adding newly accepted connection to "
                   "the dispatcher: %s",
                   _socket->getSocketDescriptor(), buffer);
    }

    _factory->release(serverSocket);

    return true;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char buffer[16];

    acceptData._peerAddress.getIPAddress(buffer, sizeof(buffer));

    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[listener:%d] Accepted new connection from %s",
                 _socket->getSocketDescriptor(), buffer);
  }

  return true;
}

bool AWSHttpListeningSocket::handleConnectEvent(ESFFlag *isRunning,
                                                ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle connect events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSHttpListeningSocket::handleReadableEvent(ESFFlag *isRunning,
                                                 ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle readable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSHttpListeningSocket::handleWritableEvent(ESFFlag *isRunning,
                                                 ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle writable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSHttpListeningSocket::handleErrorEvent(ESFError errorCode,
                                              ESFFlag *isRunning,
                                              ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    char buffer[100];

    ESFDescribeError(errorCode, buffer, sizeof(buffer));

    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Error on socket: %s",
                 _socket->getSocketDescriptor(), buffer);
  }

  return true;
}

bool AWSHttpListeningSocket::handleEndOfFileEvent(ESFFlag *isRunning,
                                                  ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle eof events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSHttpListeningSocket::handleIdleEvent(ESFFlag *isRunning,
                                             ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle idle events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSHttpListeningSocket::handleRemoveEvent(ESFFlag *flag,
                                               ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                 "[listener:%d] Removed from multiplexer",
                 _socket->getSocketDescriptor());
  }

  return true;  // call cleanup handler on us after this returns
}

SOCKET AWSHttpListeningSocket::getSocketDescriptor() const {
  return _socket ? _socket->getSocketDescriptor() : INVALID_SOCKET;
}

ESFCleanupHandler *AWSHttpListeningSocket::getCleanupHandler() {
  return _thisCleanupHandler;
}

const char *AWSHttpListeningSocket::getName() const {
  return "AWSHttpListeningSocket";
}

bool AWSHttpListeningSocket::run(ESFFlag *isRunning) {
  return false;  // todo - log
}
