#ifndef ESB_ECHO_SERVER_H
#include "ESBEchoServer.h"
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ESB {

static const UInt32 IdleTimeoutMsec = 60 * 1000U;

EchoServer::ClearListener::ClearListener(EpollMultiplexer& multiplexer, Allocator& allocator,
                                         SocketAddress::TransportType type)
    : _address("127.0.0.1", ESB_ANY_PORT, type),
      _listener("clear", _address, ESB_UINT16_MAX),
      _multiplexer(multiplexer),
      _allocator(allocator) {}

EchoServer::ClearListener::~ClearListener() {}

Error EchoServer::ClearListener::initialize() {
  Error error = _listener.bind();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot bind to ephemeral port", _listener.name());
    return error;
  }

  error = _listener.listen();
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot listen on ephemeral port", _listener.name());
    return error;
  }

  _address.setPort(_listener.listeningAddress().port());
  return ESB_SUCCESS;
}

CleanupHandler* EchoServer::ClearListener::cleanupHandler() { return NULL; }
const void* EchoServer::ClearListener::key() const { return NULL; }
bool EchoServer::ClearListener::permanent() { return true; }
bool EchoServer::ClearListener::wantAccept() { return true; }
bool EchoServer::ClearListener::wantConnect() { return false; }
bool EchoServer::ClearListener::wantRead() { return false; }
bool EchoServer::ClearListener::wantWrite() { return false; }

Error EchoServer::ClearListener::handleAccept() {
  while (true) {
    Socket::State state;
    Error error = _listener.accept(&state);
    switch (error) {
      case ESB_AGAIN:
        return ESB_SUCCESS;
      case ESB_SUCCESS:
        break;
      default:
        ESB_LOG_ERROR_ERRNO(error, "[%s] cannot accept connection", _listener.name());
        return error;
    }

    ClearEchoSocket* socket = new (_allocator) ClearEchoSocket(state, _allocator.cleanupHandler());
    if (!socket) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create new socket", _listener.name());
      Socket::Close(state.socketDescriptor());
      return ESB_OUT_OF_MEMORY;
    }

    error = _multiplexer.addMultiplexedSocket(socket);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add socket to multiplexer", socket->name());
      socket->~ClearEchoSocket();
      _allocator.deallocate(socket);
      return error;
    }
  }
}

Error EchoServer::ClearListener::handleConnect() { return ESB_NOT_IMPLEMENTED; }
Error EchoServer::ClearListener::handleReadable() { return ESB_NOT_IMPLEMENTED; }
Error EchoServer::ClearListener::handleWritable() { return ESB_NOT_IMPLEMENTED; }

void EchoServer::ClearListener::handleError(Error error) {
  ESB_LOG_ERROR_ERRNO(error, "[%s] listening socket error", _listener.name());
}

void EchoServer::ClearListener::handleRemoteClose() {}
void EchoServer::ClearListener::handleIdle() {}
void EchoServer::ClearListener::handleRemove() {}
SOCKET EchoServer::ClearListener::socketDescriptor() const { return _listener.socketDescriptor(); }
const char* EchoServer::ClearListener::name() const { return _listener.name(); }
void EchoServer::ClearListener::markDead() {}
bool EchoServer::ClearListener::dead() const { return false; }

EchoServer::SecureListener::SecureListener(EpollMultiplexer& multiplexer, Allocator& allocator,
                                           ServerTLSContextIndex& index)
    : ClearListener(multiplexer, allocator, SocketAddress::TLS), _index(index) {}
EchoServer::SecureListener::~SecureListener() {}

Error EchoServer::SecureListener::handleAccept() {
  while (true) {
    Socket::State state;
    Error error = _listener.accept(&state);
    switch (error) {
      case ESB_AGAIN:
        return ESB_SUCCESS;
      case ESB_SUCCESS:
        break;
      default:
        ESB_LOG_ERROR_ERRNO(error, "[%s] cannot accept connection", _listener.name());
        return error;
    }

    SecureEchoSocket* socket = new (_allocator) SecureEchoSocket(state, _index, _allocator.cleanupHandler());
    if (!socket) {
      ESB_LOG_ERROR_ERRNO(ESB_OUT_OF_MEMORY, "[%s] cannot create new socket", _listener.name());
      Socket::Close(state.socketDescriptor());
      return ESB_OUT_OF_MEMORY;
    }

    error = _multiplexer.addMultiplexedSocket(socket);
    if (ESB_SUCCESS != error) {
      ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add socket to multiplexer", socket->name());
      socket->~SecureEchoSocket();
      _allocator.deallocate(socket);
      return error;
    }
  }
}

EchoServer::EchoSocket::EchoSocket(CleanupHandler& cleanupHandler)
    : _dead(false), _buffer(_storage, sizeof(_storage)), _cleanupHandler(cleanupHandler) {}
EchoServer::EchoSocket::~EchoSocket() {}

CleanupHandler* EchoServer::EchoSocket::cleanupHandler() { return &_cleanupHandler; }
const void* EchoServer::EchoSocket::key() const { return NULL; }
bool EchoServer::EchoSocket::permanent() { return false; }
bool EchoServer::EchoSocket::wantAccept() { return false; }
bool EchoServer::EchoSocket::wantConnect() { return false; }
bool EchoServer::EchoSocket::wantRead() { return _buffer.isWritable(); }
bool EchoServer::EchoSocket::wantWrite() { return _buffer.isReadable(); }
Error EchoServer::EchoSocket::handleAccept() { return ESB_NOT_IMPLEMENTED; }
Error EchoServer::EchoSocket::handleConnect() { return ESB_NOT_IMPLEMENTED; }

Error EchoServer::EchoSocket::handleReadable() {
  assert(_buffer.isWritable());

  ConnectedSocket& socket = subclassSocket();

  while (_buffer.isWritable()) {
    SSize result = socket.receive(&_buffer);

    if (0 > result) {
      Error error = LastError();
      if (ESB_AGAIN != error) {
        ESB_LOG_ERROR_ERRNO(error, "[%s] cannot receive data", socket.name());
      }
      return error;
    }

    if (0 == result) {
      ESB_LOG_INFO("[%s] peer closed connection", socket.name());
      return ESB_CLOSED;
    }

    ESB_LOG_DEBUG("[%s] read %ld bytes", socket.name(), result);
  }

  //
  // The echo server will lose edge triggered notifications if it cannot fully read all data off the socket into the
  // buffer.  If you hit this assertion, increase the buffer size.
  //
  assert(_buffer.isWritable());
  return ESB_AGAIN;
}

Error EchoServer::EchoSocket::handleWritable() {
  assert(_buffer.isReadable());

  ConnectedSocket& socket = subclassSocket();

  while (_buffer.isReadable()) {
    SSize result = socket.send(&_buffer);
    if (0 > result) {
      Error error = LastError();
      if (ESB_AGAIN != error) {
        ESB_LOG_ERROR_ERRNO(error, "[%s] cannot send data", socket.name());
      }
      return error;
    }

    ESB_LOG_DEBUG("[%s] sent %ld bytes", socket.name(), result);
    _buffer.compact();
  }

  return ESB_AGAIN;
}

void EchoServer::EchoSocket::handleError(Error error) {
  ESB_LOG_ERROR_ERRNO(error, "[%s] connected socket error", subclassSocket().name());
}

void EchoServer::EchoSocket::handleRemoteClose() {
  ESB_LOG_INFO("[%s] peer closed connection", subclassSocket().name());
}

void EchoServer::EchoSocket::handleIdle() { ESB_LOG_INFO("[%s] idle timeout", subclassSocket().name()); }
void EchoServer::EchoSocket::handleRemove() { ESB_LOG_INFO("[%s] removed", subclassSocket().name()); }
SOCKET EchoServer::EchoSocket::socketDescriptor() const { return subclassSocket().socketDescriptor(); }
const char* EchoServer::EchoSocket::name() const { return subclassSocket().name(); }
void EchoServer::EchoSocket::markDead() { _dead = true; }
bool EchoServer::EchoSocket::dead() const { return _dead; }

EchoServer::ClearEchoSocket::ClearEchoSocket(const Socket::State& state, CleanupHandler& cleanupHandler)
    : EchoSocket(cleanupHandler), _socket(state, "clr-echo") {}

EchoServer::ClearEchoSocket::~ClearEchoSocket() {}

ConnectedSocket& EchoServer::ClearEchoSocket::subclassSocket() { return _socket; }

const ConnectedSocket& EchoServer::ClearEchoSocket::subclassSocket() const { return _socket; }

EchoServer::SecureEchoSocket::SecureEchoSocket(const Socket::State& state, ServerTLSContextIndex& index,
                                               CleanupHandler& cleanupHandler)
    : EchoSocket(cleanupHandler), _socket(state, "sec-echo", index) {}

EchoServer::SecureEchoSocket::~SecureEchoSocket() {}

ConnectedSocket& EchoServer::SecureEchoSocket::subclassSocket() { return _socket; }

const ConnectedSocket& EchoServer::SecureEchoSocket::subclassSocket() const { return _socket; }

EchoServer::EchoServer(Allocator& allocator)
    : _allocator(allocator),
      _contextIndex(997, 47, _allocator),
      _multiplexer("echo", IdleTimeoutMsec, SystemConfig::Instance().socketSoftMax(), _allocator),
      _clearListener(_multiplexer, _allocator),
      _secureListener(_multiplexer, _allocator, _contextIndex) {}

EchoServer::~EchoServer() {
}

Error EchoServer::initialize() {
  Error error = _clearListener.initialize();
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _secureListener.initialize();
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _multiplexer.addMultiplexedSocket(&_clearListener);
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add listener to multiplexer", _clearListener.name());
    return error;
  }

  error = _multiplexer.addMultiplexedSocket(&_secureListener);
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "[%s] cannot add listener to multiplexer", _secureListener.name());
    return error;
  }

  return ESB_SUCCESS;
}

void EchoServer::run() {
  ESB_LOG_INFO("[%s] starting", _multiplexer.name());
  _multiplexer.run(&_isRunning);
  ESB_LOG_INFO("[%s] stopping", _multiplexer.name());
}

}  // namespace ESB
