#ifndef EST_CLIENT_SOCKET_H
#include <ESTClientSocket.h>
#endif

#ifndef ESB_NULL_LOGGER_H
#include <ESBNullLogger.h>
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
ESB::BufferPool ClientSocket::_BufferPool(1024, 900,
                                          ESB::SystemAllocator::GetInstance());

ClientSocket::ClientSocket(ClientSocketFactory *factory,
                           PerformanceCounter *counter,
                           int requestsPerConnection,
                           const ESB::SocketAddress &peer,
                           ESB::CleanupHandler *cleanupHandler,
                           ESB::Logger *logger)
    : _requestsPerConnection(requestsPerConnection),
      _inReadMode(false),
      _successCounter(counter),
      _logger(logger ? logger : ESB::NullLogger::GetInstance()),
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

  return _socket.isConnected() && false == _inReadMode &&
         0 < _buffer->getReadable();
}

bool ClientSocket::isIdle()  // todo pass in current time to reduce number of
                             // syscalls
{
  return false;  // todo - implement
}

bool ClientSocket::handleAcceptEvent(ESB::Flag *isRunning,
                                     ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Warning)) {
    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Cannot handle accept events",
                 _socket.getSocketDescriptor());
  }

  return true;  // todo - keep in multiplexer
}

bool ClientSocket::handleConnectEvent(ESB::Flag *isRunning,
                                      ESB::Logger *logger) {
  if (0 == _buffer && false == setupBuffer()) {
    return false;
  }

  assert(_socket.isConnected());

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Connected", _socket.getSocketDescriptor());
  }

  assert(wantWrite());

  return handleWritableEvent(isRunning, logger);
}

bool ClientSocket::handleReadableEvent(ESB::Flag *isRunning,
                                       ESB::Logger *logger) {
  if (0 == _buffer && false == setupBuffer()) {
    return false;
  }

  assert(_buffer);
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

  _successCounter->addObservation(&_start);

  _start.tv_usec = 0;
  _start.tv_sec = 0;

  if (_logger->isLoggable(ESB::Logger::Debug)) {
    _logger->log(ESB::Logger::Debug, __FILE__, __LINE__,
                 "[socket:%d] Received complete response",
                 _socket.getSocketDescriptor());
  }

  _inReadMode = false;

  assert(wantWrite());

  if (false == _ReuseConnections) {
    return false;  // remove from multiplexer
  }

  PerformanceCounter::GetTime(&_start);

  return true;  // keep in multiplexer - also yields to other sockets
}

bool ClientSocket::handleWritableEvent(ESB::Flag *isRunning,
                                       ESB::Logger *logger) {
  if (0 == _buffer && false == setupBuffer()) {
    return false;
  }

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
                 "[socket:%d] Sent complete request",
                 _socket.getSocketDescriptor());
  }

  _inReadMode = true;
  _buffer->compact();
  assert(true == wantRead());

  return handleReadableEvent(isRunning, logger);
}

bool ClientSocket::handleErrorEvent(ESB::Error errorCode, ESB::Flag *isRunning,
                                    ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Warning)) {
    char buffer[100];
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));
    ESB::DescribeError(errorCode, buffer, sizeof(buffer));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Error from server %s: %s",
                 _socket.getSocketDescriptor(), dottedAddress, buffer);
  }

  return false;  // remove from multiplexer
}

bool ClientSocket::handleEndOfFileEvent(ESB::Flag *isRunning,
                                        ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Server %s closed socket",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool ClientSocket::handleIdleEvent(ESB::Flag *isRunning, ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Server %s is idle", _socket.getSocketDescriptor(),
                 dottedAddress);
  }

  return false;  // remove from multiplexer
}

bool ClientSocket::handleRemoveEvent(ESB::Flag *isRunning,
                                     ESB::Logger *logger) {
  if (_logger->isLoggable(ESB::Logger::Debug)) {
    char dottedAddress[16];

    _socket.getPeerAddress().getIPAddress(dottedAddress, sizeof(dottedAddress));

    _logger->log(ESB::Logger::Warning, __FILE__, __LINE__,
                 "[socket:%d] Closing socket for server %s",
                 _socket.getSocketDescriptor(), dottedAddress);
  }

  _BufferPool.releaseBuffer(_buffer);
  _buffer = 0;
  _socket.close();

  if (false == isRunning->get()) {
    return true;  // call cleanup handler on us after this returns
  }

  ESB::Error error = _factory->addNewConnection(_socket.getPeerAddress());

  if (ESB_SUCCESS != error) {
    if (logger->isLoggable(ESB::Logger::Critical)) {
      char buffer[100];

      ESB::DescribeError(error, buffer, sizeof(buffer));

      logger->log(ESB::Logger::Critical, __FILE__, __LINE__,
                  "[socket] Cannot add new connection: %s", buffer);
    }
  }

  return true;  // call cleanup handler on us after this returns
}

SOCKET ClientSocket::getSocketDescriptor() const {
  return _socket.getSocketDescriptor();
}

ESB::CleanupHandler *ClientSocket::getCleanupHandler() {
  return _cleanupHandler;
}

const char *ClientSocket::getName() const { return "ClientSocket"; }

bool ClientSocket::run(ESB::Flag *isRunning) {
  return false;  // todo - log warning
}

bool ClientSocket::setupBuffer() {
  if (_buffer) {
    return true;
  }

  _buffer = _BufferPool.acquireBuffer();

  if (!_buffer) {
    if (_logger->isLoggable(ESB::Logger::Err)) {
      _logger->log(ESB::Logger::Err, __FILE__, __LINE__,
                   "[socket:%d] Cannot acquire new buffer",
                   _socket.getSocketDescriptor());
    }

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
