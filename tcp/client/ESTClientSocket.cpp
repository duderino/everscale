#ifndef EST_CLIENT_SOCKET_H
#include <ESTClientSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

namespace EST {

// TODO buffer pool size not hardcoded
bool ClientSocket::_ReuseConnections = true;
ESB::BufferPool ClientSocket::_BufferPool(1024, 900, ESB::SystemAllocator::GetInstance());

ClientSocket::ClientSocket(ClientSocketFactory *factory, PerformanceCounter *counter, int requestsPerConnection,
                           const ESB::SocketAddress &peer, ESB::CleanupHandler *cleanupHandler)
    : _requestsPerConnection(requestsPerConnection),
      _inReadMode(false),
      _successCounter(counter),
      _cleanupHandler(cleanupHandler),
      _buffer(0),
      _socket(peer, false),
      _factory(factory) {
  setupBuffer();
  PerformanceCounter::GetTime(&_start);
}

ClientSocket::~ClientSocket() {}

bool ClientSocket::wantAccept() { return false; }

bool ClientSocket::wantConnect() { return false == _socket.isConnected(); }

bool ClientSocket::wantRead() {
  if (0 == _buffer && false == setupBuffer()) {
    return false;
  }

  return _socket.isConnected() && _inReadMode && 0 < _buffer->getWritable();
}

bool ClientSocket::wantWrite() {
  if (0 == _buffer && false == setupBuffer()) {
    return false;
  }

  return _socket.isConnected() && false == _inReadMode && 0 < _buffer->getReadable();
}

bool ClientSocket::isIdle() {
  return false;  // todo - implement
}

bool ClientSocket::handleAcceptEvent(ESB::SocketMultiplexer &multiplexer) {
  ESB_LOG_WARNING("[socket:%d] Cannot handle accept events", _socket.getSocketDescriptor());
  return true;  // keep in multiplexer
}

bool ClientSocket::handleConnectEvent(ESB::SocketMultiplexer &multiplexer) {
  if (0 == _buffer && !setupBuffer()) {
    return false;
  }

  assert(_socket.isConnected());
  ESB_LOG_INFO("[socket:%d] Connected", _socket.getSocketDescriptor());
  assert(wantWrite());

  return handleWritableEvent(multiplexer);
}

bool ClientSocket::handleReadableEvent(ESB::SocketMultiplexer &multiplexer) {
  if (0 == _buffer && !setupBuffer()) {
    return false;
  }

  assert(_buffer);
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

  _successCounter->addObservation(&_start);
  _start.tv_usec = 0;
  _start.tv_sec = 0;
  ESB_LOG_DEBUG("[socket:%d] Received complete response", _socket.getSocketDescriptor());

  _inReadMode = false;
  assert(wantWrite());

  if (!_ReuseConnections) {
    return false;  // remove from multiplexer
  }

  PerformanceCounter::GetTime(&_start);
  return true;  // keep in multiplexer - also yields to other sockets
}

bool ClientSocket::handleWritableEvent(ESB::SocketMultiplexer &multiplexer) {
  if (0 == _buffer && !setupBuffer()) {
    return false;
  }

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

  ESB_LOG_DEBUG("[socket:%d] Sent complete request", _socket.getSocketDescriptor());

  _inReadMode = true;
  _buffer->compact();
  assert(wantRead());
  return handleReadableEvent(multiplexer);
}

bool ClientSocket::handleErrorEvent(ESB::Error error, ESB::SocketMultiplexer &multiplexer) {
  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO_ERRNO(error, "[socket:%d] Error from server %s:%d", _socket.getSocketDescriptor(), dottedAddress,
                       _socket.getPeerAddress().getPort());
  }
  return false;  // remove from multiplexer
}

bool ClientSocket::handleEndOfFileEvent(ESB::SocketMultiplexer &multiplexer) {
  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("[socket:%d] Server %s:%d closed socket", _socket.getSocketDescriptor(), dottedAddress,
                 _socket.getPeerAddress().getPort());
  }
  return false;  // remove from multiplexer
}

bool ClientSocket::handleIdleEvent(ESB::SocketMultiplexer &multiplexer) {
  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("[socket:%d] Server %s:%d is idle", _socket.getSocketDescriptor(), dottedAddress,
                 _socket.getPeerAddress().getPort());
  }
  return false;  // remove from multiplexer
}

bool ClientSocket::handleRemoveEvent(ESB::SocketMultiplexer &multiplexer) {
  if (ESB_INFO_LOGGABLE) {
    char dottedAddress[ESB_IPV6_PRESENTATION_SIZE];
    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB_LOG_INFO("[socket:%d] Socket to server %s:%d closed", _socket.getSocketDescriptor(), dottedAddress,
                 _socket.getPeerAddress().getPort());
  }

  _BufferPool.releaseBuffer(_buffer);
  _buffer = 0;
  _socket.close();

  if (!multiplexer.isRunning()) {
    return true;  // call cleanup handler on us after this returns
  }

  ESB::Error error = _factory->addNewConnection(_socket.getPeerAddress());

  if (ESB_SUCCESS != error) {
    ESB_LOG_WARNING_ERRNO(error, "[socket] Cannot add new connection");
  }

  return true;  // call cleanup handler on us after this returns
}

SOCKET ClientSocket::getSocketDescriptor() const { return _socket.getSocketDescriptor(); }

ESB::CleanupHandler *ClientSocket::getCleanupHandler() { return _cleanupHandler; }

const char *ClientSocket::getName() const { return "ClientSocket"; }

bool ClientSocket::setupBuffer() {
  if (_buffer) {
    return true;
  }

  _buffer = _BufferPool.acquireBuffer();

  if (!_buffer) {
    ESB_LOG_ERROR("[socket:%d] Cannot acquire new buffer", _socket.getSocketDescriptor());
    return false;
  }

  assert(_buffer->isWritable());
  _buffer->clear();
  unsigned int capacity = _buffer->getCapacity();
  memset(_buffer->getBuffer(), ESB::Thread::GetThreadId() % 256, capacity);
  _buffer->setWritePosition(capacity);

  _inReadMode = false;
  assert(capacity = _buffer->getReadable());
  assert(0 == _buffer->getWritable());

  return true;
}

}  // namespace EST
