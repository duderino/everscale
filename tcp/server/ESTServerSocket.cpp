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
ESB::BufferPool ServerSocket::_BufferPool(1024, 900, ESB::SystemAllocator::GetInstance());

ServerSocket::ServerSocket(ESB::TCPSocket::AcceptData *acceptData, ESB::CleanupHandler *cleanupHandler)
    : _hasBeenRemoved(false), _inReadMode(true), _cleanupHandler(cleanupHandler), _buffer(0), _socket(acceptData) {}

ServerSocket::~ServerSocket() {}

bool ServerSocket::wantAccept() {
  assert(!_hasBeenRemoved);
  return false;
}

bool ServerSocket::wantConnect() {
  assert(!_hasBeenRemoved);
  return false;
}

bool ServerSocket::wantRead() {
  assert(!_hasBeenRemoved);
  if (!_buffer) {
    return true;
  }
  return _inReadMode && _buffer->isWritable();
}

bool ServerSocket::wantWrite() {
  assert(!_hasBeenRemoved);
  if (!_buffer) {
    return false;
  }
  return !_inReadMode && _buffer->isReadable();
}

bool ServerSocket::isIdle() {
  assert(!_hasBeenRemoved);
  return false;  // todo - implement
}

bool ServerSocket::handleAcceptEvent(ESB::SocketMultiplexer &multiplexer) {
  assert(!_hasBeenRemoved);
  ESB_LOG_WARNING("[socket:%d] Cannot handle accept events", _socket.getSocketDescriptor());
  return true;  // keep in multiplexer
}

bool ServerSocket::handleConnectEvent(ESB::SocketMultiplexer &multiplexer) {
  assert(!_hasBeenRemoved);
  ESB_LOG_WARNING("[socket:%d] Cannot handle connect events", _socket.getSocketDescriptor());
  return true;  // keep in multiplexer
}

bool ServerSocket::handleReadableEvent(ESB::SocketMultiplexer &multiplexer) {
  assert(!_hasBeenRemoved);

  if (!_buffer) {
    _buffer = _BufferPool.acquireBuffer();
    if (!_buffer) {
      ESB_LOG_WARNING("[socket:%d] Cannot acquire new buffer", _socket.getSocketDescriptor());
      return false;  // remove from multiplexer
    }
  }

  assert(_inReadMode);
  assert(_buffer->isWritable());

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (multiplexer.isRunning() && _buffer->isWritable()) {
    result = _socket.receive(_buffer);

    if (0 > result) {
      error = ESB::GetLastError();

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("[socket:%d] not ready for read", _socket.getSocketDescriptor());
        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        ESB_LOG_DEBUG("[socket:%d] interrupted", _socket.getSocketDescriptor());
        continue;
      }

      return handleErrorEvent(error, multiplexer);
    }

    if (0 == result) {
      return handleEndOfFileEvent(multiplexer);
    }

    ESB_LOG_DEBUG("[socket:%d] Read %ld bytes", _socket.getSocketDescriptor(), result);
  }

  if (!multiplexer.isRunning()) {
    return false;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[socket:%d] Received complete request", _socket.getSocketDescriptor());

  _inReadMode = false;
  assert(wantWrite());
  return handleWritableEvent(multiplexer);
}

bool ServerSocket::handleWritableEvent(ESB::SocketMultiplexer &multiplexer) {
  assert(!_hasBeenRemoved);

  assert(_buffer);
  assert(!_inReadMode);
  assert(_buffer->isReadable());

  ESB::SSize result = 0;
  ESB::Error error = ESB_SUCCESS;

  while (multiplexer.isRunning() && _buffer->isReadable()) {
    result = _socket.send(_buffer);

    if (0 > result) {
      error = ESB::GetLastError();

      if (ESB_AGAIN == error) {
        ESB_LOG_DEBUG("[socket:%d] Not ready for write", _socket.getSocketDescriptor());
        return true;  // keep in multiplexer
      }

      if (ESB_INTR == error) {
        ESB_LOG_DEBUG("[socket:%d] Interrupted", _socket.getSocketDescriptor());
        continue;
      }

      return handleErrorEvent(error, multiplexer);
    }

    ESB_LOG_DEBUG("[socket:%d] Wrote %ld bytes", _socket.getSocketDescriptor(), result);
  }

  if (!multiplexer.isRunning()) {
    return false;  // remove from multiplexer
  }

  ESB_LOG_DEBUG("[socket:%d] Sent complete response", _socket.getSocketDescriptor());

  _inReadMode = true;
  _buffer->compact();
  _BufferPool.releaseBuffer(_buffer);
  _buffer = 0;

  return true;  // keep in multiplexer
}

bool ServerSocket::handleErrorEvent(ESB::Error error, ESB::SocketMultiplexer &multiplexer) {
  assert(!_hasBeenRemoved);

  if (ESB_WARNING_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_WARNING_ERRNO(error, "[socket:%d] Error from client %s:%d", _socket.getSocketDescriptor(), dottedAddress,
                          _socket.getPeerAddress().getPort());
  }

  return false;  // remove from multiplexer
}

bool ServerSocket::handleEndOfFileEvent(ESB::SocketMultiplexer &multiplexer) {
  assert(!_hasBeenRemoved);

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("[socket:%d] client at %s:%d closed socket", _socket.getSocketDescriptor(), dottedAddress,
                 _socket.getPeerAddress().getPort());
  }

  return false;  // remove from multiplexer
}

bool ServerSocket::handleIdleEvent(ESB::SocketMultiplexer &multiplexer) {
  assert(!_hasBeenRemoved);

  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("[socket:%d] client at %s:%d is idle", _socket.getSocketDescriptor(), dottedAddress,
                 _socket.getPeerAddress().getPort());
  }

  return false;  // remove from multiplexer
}

bool ServerSocket::handleRemoveEvent(ESB::SocketMultiplexer &multiplexer) {
  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("[socket:%d] socket to %s:%d is closed", _socket.getSocketDescriptor(), dottedAddress,
                 _socket.getPeerAddress().getPort());
  }

  _socket.close();
  _hasBeenRemoved = true;

  return true;  // call cleanup handler on us after this returns
}

SOCKET ServerSocket::getSocketDescriptor() const { return _socket.getSocketDescriptor(); }

ESB::CleanupHandler *ServerSocket::getCleanupHandler() { return _cleanupHandler; }

const char *ServerSocket::getName() const { return "ServerSocket"; }

}  // namespace EST
