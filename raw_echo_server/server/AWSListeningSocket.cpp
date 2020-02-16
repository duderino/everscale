/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_LISTENING_SOCKET_H
#include <AWSListeningSocket.h>
#endif

#ifndef AWS_SERVER_SOCKET_H
#include <AWSServerSocket.h>
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

AWSListeningSocket::AWSListeningSocket(
    ESFListeningTCPSocket *socket, ESFAllocator *allocator,
    ESFSocketMultiplexerDispatcher *dispatcher, ESFLogger *logger,
    ESFCleanupHandler *thisCleanupHandler,
    ESFCleanupHandler *socketCleanupHandler)
    : _socket(socket),
      _allocator(allocator ? allocator : ESFSystemAllocator::GetInstance()),
      _dispatcher(dispatcher),
      _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _thisCleanupHandler(thisCleanupHandler),
      _socketCleanupHandler(socketCleanupHandler) {}

AWSListeningSocket::~AWSListeningSocket() {
  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "AWSListeningSocket destroyed");
  }
}

bool AWSListeningSocket::wantAccept() { return true; }

bool AWSListeningSocket::wantConnect() { return false; }

bool AWSListeningSocket::wantRead() { return false; }

bool AWSListeningSocket::wantWrite() { return false; }

bool AWSListeningSocket::isIdle() { return false; }

bool AWSListeningSocket::handleAcceptEvent(ESFFlag *isRunning,
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

  AWSServerSocket *serverSocket = new (_allocator)
      AWSServerSocket(&acceptData, _socketCleanupHandler, _logger);

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

    _socketCleanupHandler->destroy(serverSocket);

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

    _socketCleanupHandler->destroy(serverSocket);

    return true;
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char buffer[16];

    acceptData._peerAddress.getIPAddress(buffer, sizeof(buffer));

    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Accepted new connection from %s",
                 _socket->getSocketDescriptor(), buffer);
  }

  return true;
}

bool AWSListeningSocket::handleConnectEvent(ESFFlag *isRunning,
                                            ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle connect events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSListeningSocket::handleReadableEvent(ESFFlag *isRunning,
                                             ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle readable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSListeningSocket::handleWritableEvent(ESFFlag *isRunning,
                                             ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle writable events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSListeningSocket::handleErrorEvent(ESFError errorCode,
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

bool AWSListeningSocket::handleEndOfFileEvent(ESFFlag *isRunning,
                                              ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle eof events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSListeningSocket::handleIdleEvent(ESFFlag *isRunning,
                                         ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Error)) {
    _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                 "[listener:%d] Cannot handle idle events",
                 _socket->getSocketDescriptor());
  }

  return true;
}

bool AWSListeningSocket::handleRemoveEvent(ESFFlag *flag, ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Notice)) {
    _logger->log(ESFLogger::Notice, __FILE__, __LINE__,
                 "[listener:%d] Removed from multiplexer",
                 _socket->getSocketDescriptor());
  }

  return true;  // call cleanup handler on us after this returns
}

SOCKET AWSListeningSocket::getSocketDescriptor() const {
  return _socket ? _socket->getSocketDescriptor() : INVALID_SOCKET;
}

ESFCleanupHandler *AWSListeningSocket::getCleanupHandler() {
  return _thisCleanupHandler;
}

const char *AWSListeningSocket::getName() const { return "AWSListeningSocket"; }

bool AWSListeningSocket::run(ESFFlag *isRunning) {
  return false;  // todo - log
}
