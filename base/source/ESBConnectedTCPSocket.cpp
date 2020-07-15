#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#if defined HAVE_FCNTL_H && defined USE_FCNTL_FOR_NONBLOCK
#include <fcntl.h>
#endif

namespace ESB {

#define ESB_SOCK_IS_CONNECTED (1 << 0)

ConnectedTCPSocket::ConnectedTCPSocket(const char *namePrefix, const char *nameSuffix, bool isBlocking)
    : TCPSocket(isBlocking), _flags(0), _localAddress(), _peerAddress() {
  formatPrefix(namePrefix, nameSuffix);
}

ConnectedTCPSocket::ConnectedTCPSocket(const char *namePrefix, const char *nameSuffix, const SocketAddress &peer,
                                       bool isBlocking)
    : TCPSocket(isBlocking), _flags(0), _localAddress(), _peerAddress(peer) {
  formatPrefix(namePrefix, nameSuffix);
}

ConnectedTCPSocket::~ConnectedTCPSocket() {}

const char *ConnectedTCPSocket::name() const { return _logAddress; }

Error ConnectedTCPSocket::reset(const State &acceptData) {
  Error error = TCPSocket::reset(acceptData);

  if (ESB_SUCCESS != error) {
    return error;
  }

  _flags |= ESB_SOCK_IS_CONNECTED;

  _peerAddress = acceptData.peerAddress();
  error = updateLocalAddress();
  if (ESB_SUCCESS != error) {
    close();
    return error;
  }
  updateName();

  return ESB_SUCCESS;
}

ESB::Error ConnectedTCPSocket::updateLocalAddress() {
  SocketAddress::Address address;

#if defined HAVE_GETSOCKNAME
  socklen_t socklen = sizeof(address);
  if (SOCKET_ERROR == ::getsockname(_sockFd, (sockaddr *)&address, &socklen)) {
    close();
    return LastError();
  }
#else
#error "getsockname or equivalent is required"
#endif

  _localAddress.updatePrimitiveAddress(&address);

  return ESB_SUCCESS;
}

void ConnectedTCPSocket::updateName() {
  char *p = _logAddress;
  for (; *p && *p != ':'; ++p) {
  }

  if (!*p) {
    return;
  }

  ++p;  // skip ':'
  p += _localAddress.logAddress(p, sizeof(_logAddress) - (p - _logAddress), INVALID_SOCKET);
  *p++ = '-';
  _peerAddress.logAddress(p, sizeof(_logAddress) - (p - _logAddress), _sockFd);
}

const SocketAddress &ConnectedTCPSocket::peerAddress() const { return _peerAddress; }

const SocketAddress &ConnectedTCPSocket::localAddress() const { return _localAddress; }

Error ConnectedTCPSocket::connect() {
  Error error = ESB_SUCCESS;

  if (INVALID_SOCKET != _sockFd) {
    return ESB_INVALID_STATE;
  }

#ifdef HAVE_SOCKET
  _sockFd = socket(AF_INET, SOCK_STREAM, 0);
#else
#error "socket or equivalent is required"
#endif

  if (INVALID_SOCKET == _sockFd) {
    return LastError();
  }

  error = setBlocking(_isBlocking);

  if (ESB_SUCCESS != error) {
    close();
    return error;
  }

#if defined HAVE_CONNECT && defined HAVE_STRUCT_SOCKADDR

  if (SOCKET_ERROR == ::connect(_sockFd, (sockaddr *)_peerAddress.primitiveAddress(), sizeof(SocketAddress::Address))) {
    error = LastError();

    if (!_isBlocking) {
#if defined UNIX_NONBLOCKING_CONNECT_ERROR
      if (ESB_INPROGRESS == error) {
        error = updateLocalAddress();
        if (ESB_SUCCESS != error) {
          close();
          return error;
        }
        updateName();
        return ESB_SUCCESS;
      }
#elif defined WIN32_NONBLOCKING_CONNECT_ERROR
      error = updateLocalAddress();
      if (ESB_SUCCESS != error) {
        close();
        return error;
      }
      updateName();
      return ESB_SUCCESS;
    }
#else
#error "Non-blocking connect error codes must be handled"
#endif
    }

    close();

    return error;
  }
#else
#error "connect and sockaddr or equivalent is required"
#endif

  error = updateLocalAddress();
  if (ESB_SUCCESS != error) {
    close();
    return error;
  }
  updateName();

  _flags |= ESB_SOCK_IS_CONNECTED;

  return ESB_SUCCESS;
}  // namespace ESB

void ConnectedTCPSocket::close() {
  TCPSocket::close();

  _flags &= ~ESB_SOCK_IS_CONNECTED;
}

bool ConnectedTCPSocket::isConnected() {
  if (INVALID_SOCKET == _sockFd) {
    return false;
  }

  if (_flags & ESB_SOCK_IS_CONNECTED) {
    return true;
  }

  SocketAddress::Address address;

#ifdef HAVE_SOCKLEN_T
  socklen_t addressSize = sizeof(SocketAddress::Address);
#else
  ESB::UInt32 addressSize = sizeof(SocketAddress::Address);
#endif

#if defined HAVE_GETPEERNAME
  if (SOCKET_ERROR != getpeername(_sockFd, (sockaddr *)&address, &addressSize)) {
    _flags |= ESB_SOCK_IS_CONNECTED;
  }
#else
#error "getpeername or equivalent is required"
#endif

  return _flags & ESB_SOCK_IS_CONNECTED;
}

bool ConnectedTCPSocket::isClient() const { return 0 != _localAddress.port(); }

SSize ConnectedTCPSocket::receive(char *buffer, Size bufferSize) {
#if defined HAVE_RECV
  return recv(_sockFd, buffer, bufferSize, 0);
#else
#error "recv or equivalent is required."
#endif
}

SSize ConnectedTCPSocket::receive(Buffer *buffer) {
  SSize size = receive((char *)buffer->buffer() + buffer->writePosition(), buffer->writable());

  if (0 >= size) {
    return size;
  }

  buffer->setWritePosition(buffer->writePosition() + size);

  return size;
}

SSize ConnectedTCPSocket::send(const char *buffer, Size bufferSize) {
#if defined HAVE_SEND
  return ::send(_sockFd, buffer, bufferSize, 0);
#else
#error "send or equivalent is required."
#endif
}

SSize ConnectedTCPSocket::send(Buffer *buffer) {
  Size size = send((char *)buffer->buffer() + buffer->readPosition(), buffer->readable());

  if (0 >= size) {
    return size;
  }

  buffer->setReadPosition(buffer->readPosition() + size);
  buffer->compact();

  return size;
}

int ConnectedTCPSocket::bytesReadable() { return BytesReadable(_sockFd); }

int ConnectedTCPSocket::BytesReadable(SOCKET socketDescriptor) {
  if (INVALID_SOCKET == socketDescriptor) {
    return 0;
  }

#if defined HAVE_IOCTL
  int bytesReadable = 0;

  if (SOCKET_ERROR == ioctl(socketDescriptor, FIONREAD, &bytesReadable)) {
    return SOCKET_ERROR;
  }
#elif defined HAVE_IOCTLSOCKET
  unsigned long bytesReadable = 0;

  if (SOCKET_ERROR == ioctlsocket(socketDescriptor, FIONREAD, &bytesReadable)) {
    return SOCKET_ERROR;
  }
#else
#error "method to get bytes readable required"
#endif

  return bytesReadable;
}

void ConnectedTCPSocket::formatPrefix(const char *namePrefix, const char *nameSuffix) {
  int desired = snprintf(_logAddress, ESB_NAME_PREFIX_SIZE, "%s-%s:", namePrefix, nameSuffix);
  _logAddress[MIN(ESB_NAME_PREFIX_SIZE - 2, desired - 1)] = ':';
  _logAddress[MIN(ESB_NAME_PREFIX_SIZE - 1, desired)] = 0;
}

}  // namespace ESB
