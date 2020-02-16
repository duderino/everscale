/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_SERVER_SOCKET_H
#include <AWSServerSocket.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

// TODO buffer pool size not hardcoded
ESFBufferPool AWSServerSocket::_BufferPool(1024, 900,
                                           ESFSystemAllocator::GetInstance());

AWSServerSocket::AWSServerSocket(ESFTCPSocket::AcceptData *acceptData,
                                 ESFCleanupHandler *cleanupHandler,
                                 ESFLogger *logger)
    : _hasBeenRemoved(false),
      _inReadMode(true),
      _logger(logger ? logger : ESFNullLogger::GetInstance()),
      _cleanupHandler(cleanupHandler),
      _buffer(0),
      _socket(acceptData) {}

AWSServerSocket::~AWSServerSocket() {}

bool AWSServerSocket::wantAccept() {
  ESF_ASSERT(false == _hasBeenRemoved);

  return false;
}

bool AWSServerSocket::wantConnect() {
  ESF_ASSERT(false == _hasBeenRemoved);

  return false;
}

bool AWSServerSocket::wantRead() {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (0 == _buffer) {
    return true;
  }

  return _inReadMode && _buffer->isWritable();
}

bool AWSServerSocket::wantWrite() {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (0 == _buffer) {
    return false;
  }

  return false == _inReadMode && _buffer->isReadable();
}

bool AWSServerSocket::isIdle()  // todo pass in current time to reduce number of
                                // syscalls
{
  ESF_ASSERT(false == _hasBeenRemoved);

  return false;  // todo - implement
}

bool AWSServerSocket::handleAcceptEvent(ESFFlag *isRunning, ESFLogger *logger) {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESFLogger::Warning)) {
    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle accept events",
                 _socket.getSocketDescriptor());
  }

  return true;  // keep in multiplexer
}

bool AWSServerSocket::handleConnectEvent(ESFFlag *isRunning,
                                         ESFLogger *logger) {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESFLogger::Warning)) {
    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle connect events",
                 _socket.getSocketDescriptor());
  }

  return true;  // keep in multiplexer
}

bool AWSServerSocket::handleReadableEvent(ESFFlag *isRunning,
                                          ESFLogger *logger) {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (0 == _buffer) {
    _buffer = _BufferPool.acquireBuffer();

    if (!_buffer) {
      if (_logger->isLoggable(ESFLogger::Error)) {
        _logger->log(ESFLogger::Error, __FILE__, __LINE__,
                     "[socket:%d] Cannot acquire new buffer",
                     _socket.getSocketDescriptor());
      }

      return false;  // remove from multiplexer
    }
  }

  ESF_ASSERT(_inReadMode);
  ESF_ASSERT(_buffer->isWritable());

  ESFSSize result = 0;
  ESFError error = ESF_SUCCESS;

  while (isRunning->get() && _buffer->isWritable()) {
    result = _socket.receive(_buffer);

    if (0 > result) {
      error = ESFGetLastError();

      if (ESF_AGAIN == error) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] not ready for read",
                       _socket.getSocketDescriptor());
        }

        return true;  // keep in multiplexer
      }

      if (ESF_INTR == error) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] interrupted",
                       _socket.getSocketDescriptor());
        }

        continue;
      }

      return handleErrorEvent(error, isRunning, logger);
    }

    if (0 == result) {
      return handleEndOfFileEvent(isRunning, logger);
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] Read %d bytes", _socket.getSocketDescriptor(),
                   result);
    }
  }

  if (!isRunning->get()) {
    return false;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Received complete request",
                 _socket.getSocketDescriptor());
  }

  _inReadMode = false;

  ESF_ASSERT(wantWrite());

  return handleWritableEvent(isRunning, logger);
}

bool AWSServerSocket::handleWritableEvent(ESFFlag *isRunning,
                                          ESFLogger *logger) {
  ESF_ASSERT(false == _hasBeenRemoved);

  ESF_ASSERT(_buffer);
  ESF_ASSERT(false == _inReadMode);
  ESF_ASSERT(_buffer->isReadable());

  ESFSSize result = 0;
  ESFError error = ESF_SUCCESS;

  while (isRunning->get() && _buffer->isReadable()) {
    result = _socket.send(_buffer);

    if (0 > result) {
      error = ESFGetLastError();

      if (ESF_AGAIN == error) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] Not ready for write",
                       _socket.getSocketDescriptor());
        }

        return true;  // keep in multiplexer
      }

      if (ESF_INTR == error) {
        if (_logger->isLoggable(ESFLogger::Debug)) {
          _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                       "[socket:%d] Interrupted",
                       _socket.getSocketDescriptor());
        }

        continue;
      }

      return handleErrorEvent(error, isRunning, logger);
    }

    if (_logger->isLoggable(ESFLogger::Debug)) {
      _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                   "[socket:%d] Wrote %d bytes", _socket.getSocketDescriptor(),
                   result);
    }
  }

  if (!isRunning->get()) {
    return false;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESFLogger::Debug)) {
    _logger->log(ESFLogger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Sent complete response",
                 _socket.getSocketDescriptor());
  }

  _inReadMode = true;
  _buffer->compact();

  _BufferPool.releaseBuffer(_buffer);
  _buffer = 0;

  return true;  // keep in multiplexer
}

bool AWSServerSocket::handleErrorEvent(ESFError errorCode, ESFFlag *isRunning,
                                       ESFLogger *logger) {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESFLogger::Warning)) {
    char buffer[100];
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESFDescribeError(errorCode, buffer, sizeof(buffer));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Error from client %s: %s",
                 _socket.getSocketDescriptor(), dottedAddress, buffer);
  }

  return false;  // remove from multiplexer
}

bool AWSServerSocket::handleEndOfFileEvent(ESFFlag *isRunning,
                                           ESFLogger *logger) {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Client %s closed socket",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool AWSServerSocket::handleIdleEvent(ESFFlag *isRunning, ESFLogger *logger) {
  ESF_ASSERT(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Client %s is idle", _socket.getSocketDescriptor(),
                 dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool AWSServerSocket::handleRemoveEvent(ESFFlag *isRunning, ESFLogger *logger) {
  if (_logger->isLoggable(ESFLogger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESFLogger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Closing socket for client %s",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  _socket.close();
  _hasBeenRemoved = true;

  return true;  // call cleanup handler on us after this returns
}

SOCKET AWSServerSocket::getSocketDescriptor() const {
  return _socket.getSocketDescriptor();
}

ESFCleanupHandler *AWSServerSocket::getCleanupHandler() {
  return _cleanupHandler;
}

const char *AWSServerSocket::getName() const { return "AWSServerSocket"; }

bool AWSServerSocket::run(ESFFlag *isRunning) {
  return false;  // todo - log warning
}
