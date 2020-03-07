#ifndef EST_SERVER_SOCKET_H
#include <ESTServerSocket.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace EST {

// TODO buffer pool size not hardcoded
ESB::BufferPool ServerSocket::_BufferPool(1024, 900,
                                          ESB::SystemAllocator::GetInstance());

ServerSocket::ServerSocket(ESB::TCPSocket::AcceptData *acceptData,
                           ESB::CleanupHandler *cleanupHandler,
                           ESB::Logger *logger)
    : _hasBeenRemoved(false),
      _inReadMode(true),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
      _cleanupHandler(cleanupHandler),
      _buffer(0),
      _socket(acceptData) {}

ServerSocket::~ServerSocket() {}

bool ServerSocket::wantAccept() {
  assert(false == _hasBeenRemoved);

  return false;
}

bool ServerSocket::wantConnect() {
  assert(false == _hasBeenRemoved);

  return false;
}

bool ServerSocket::wantRead() {
  assert(false == _hasBeenRemoved);

  if (0 == _buffer) {
    return true;
  }

  return _inReadMode && _buffer->isWritable();
}

bool ServerSocket::wantWrite() {
  assert(false == _hasBeenRemoved);

  if (0 == _buffer) {
    return false;
  }

  return false == _inReadMode && _buffer->isReadable();
}

bool ServerSocket::isIdle()  // todo pass in current time to reduce number of
                             // syscalls
{
  assert(false == _hasBeenRemoved);

  return false;  // todo - implement
}

bool ServerSocket::handleAcceptEvent(ESB::SharedInt *isRunning,
                                     ESB::Logger *logger) {
  assert(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESB::Logger::Warning)) {
    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle accept events",
                 _socket.getSocketDescriptor());
  }

  return true;  // keep in multiplexer
}

bool ServerSocket::handleConnectEvent(ESB::SharedInt *isRunning,
                                      ESB::Logger *logger) {
  assert(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESB::Logger::Warning)) {
    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle connect events",
                 _socket.getSocketDescriptor());
  }

  return true;  // keep in multiplexer
}

bool ServerSocket::handleReadableEvent(ESB::SharedInt *isRunning,
                                       ESB::Logger *logger) {
  assert(false == _hasBeenRemoved);

  if (0 == _buffer) {
    _buffer = _BufferPool.acquireBuffer();

    if (!_buffer) {
      if (_logger->isLoggable(ESB::Logger::Err)) {
        _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                     "[socket:%d] Cannot acquire new buffer",
                     _socket.getSocketDescriptor());
      }

      return false;  // remove from multiplexer
    }
  }

  assert(_inReadMode);
  assert(_buffer->isWritable());

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (isRunning->get() && _buffer->isWritable()) {
    result = _socket.receive(_buffer);

    if (0 > result) {
      error = ESB::GetLastError();

      if (ESB_AGAIN == error) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] not ready for read",
                       _socket.getSocketDescriptor());
        }

        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
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

    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] Read %d bytes", _socket.getSocketDescriptor(),
                   result);
    }
  }

  if (!isRunning->get()) {
    return false;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Received complete request",
                 _socket.getSocketDescriptor());
  }

  _inReadMode = false;

  assert(wantWrite());

  return handleWritableEvent(isRunning, logger);
}

bool ServerSocket::handleWritableEvent(ESB::SharedInt *isRunning,
                                       ESB::Logger *logger) {
  assert(false == _hasBeenRemoved);

  assert(_buffer);
  assert(false == _inReadMode);
  assert(_buffer->isReadable());

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (isRunning->get() && _buffer->isReadable()) {
    result = _socket.send(_buffer);

    if (0 > result) {
      error = ESB::GetLastError();

      if (ESB_AGAIN == error) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] Not ready for write",
                       _socket.getSocketDescriptor());
        }

        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        if (_logger->isLoggable(ESB::Logger::Debug)) {
          _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                       "[socket:%d] Interrupted",
                       _socket.getSocketDescriptor());
        }

        continue;
      }

      return handleErrorEvent(error, isRunning, logger);
    }

    if (_logger->isLoggable(ESB::Logger::Debug)) {
      _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                   "[socket:%d] Wrote %d bytes", _socket.getSocketDescriptor(),
                   result);
    }
  }

  if (!isRunning->get()) {
    return false;  // remove from multiplexer
  }

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Sent complete response",
                 _socket.getSocketDescriptor());
  }

  _inReadMode = true;
  _buffer->compact();

  _BufferPool.releaseBuffer(_buffer);
  _buffer = 0;

  return true;  // keep in multiplexer
}

bool ServerSocket::handleErrorEvent(ESB::Error errorCode,
                                    ESB::SharedInt *isRunning,
                                    ESB::Logger *logger) {
  assert(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESB::Logger::Warning)) {
    char buffer[100];
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB::DescribeError(errorCode, buffer, sizeof(buffer));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Error from client %s: %s",
                 _socket.getSocketDescriptor(), dottedAddress, buffer);
  }

  return false;  // remove from multiplexer
}

bool ServerSocket::handleEndOfFileEvent(ESB::SharedInt *isRunning,
                                        ESB::Logger *logger) {
  assert(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Client %s closed socket",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool ServerSocket::handleIdleEvent(ESB::SharedInt *isRunning,
                                   ESB::Logger *logger) {
  assert(false == _hasBeenRemoved);

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Client %s is idle", _socket.getSocketDescriptor(),
                 dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool ServerSocket::handleRemoveEvent(ESB::SharedInt *isRunning,
                                     ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Closing socket for client %s",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  _socket.close();
  _hasBeenRemoved = true;

  return true;  // call cleanup handler on us after this returns
}

SOCKET ServerSocket::getSocketDescriptor() const {
  return _socket.getSocketDescriptor();
}

ESB::CleanupHandler *ServerSocket::getCleanupHandler() {
  return _cleanupHandler;
}

const char *ServerSocket::getName() const { return "ServerSocket"; }

bool ServerSocket::run(ESB::SharedInt *isRunning) {
  return false;  // todo - log warning
}

}  // namespace EST
