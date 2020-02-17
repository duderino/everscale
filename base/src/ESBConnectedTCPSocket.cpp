#ifndef ESB_CONNECTED_TCP_SOCKET_H
#include <ESBConnectedTCPSocket.h>
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

ConnectedTCPSocket::ConnectedTCPSocket()
    : TCPSocket(), _isConnected(false), _listenerAddress(), _peerAddress() {}

ConnectedTCPSocket::ConnectedTCPSocket(bool isBlocking)
    : TCPSocket(isBlocking),
      _isConnected(false),
      _listenerAddress(),
      _peerAddress() {}

ConnectedTCPSocket::ConnectedTCPSocket(const SocketAddress &peer,
                                       bool isBlocking)
    : TCPSocket(isBlocking),
      _isConnected(false),
      _listenerAddress(),
      _peerAddress(peer) {}

ConnectedTCPSocket::ConnectedTCPSocket(AcceptData *acceptData)
    : TCPSocket(acceptData),
      _isConnected(true),
      _listenerAddress(acceptData->_listeningAddress),
      _peerAddress(acceptData->_peerAddress) {}

ConnectedTCPSocket::~ConnectedTCPSocket() {}

Error ConnectedTCPSocket::reset(AcceptData *acceptData) {
  if (!acceptData) {
    return ESB_NULL_POINTER;
  }

  Error error = TCPSocket::reset(acceptData);

  if (ESB_SUCCESS != error) {
    return error;
  }

  _isConnected = true;
  _listenerAddress = acceptData->_listeningAddress;
  _peerAddress = acceptData->_peerAddress;

  return ESB_SUCCESS;
}

void ConnectedTCPSocket::setPeerAddress(const SocketAddress &address) {
  assert(INVALID_SOCKET == _sockFd);

  _peerAddress = address;
}

const SocketAddress &ConnectedTCPSocket::getPeerAddress() const {
  return _peerAddress;
}

const SocketAddress &ConnectedTCPSocket::getListenerAddress() const {
  return _listenerAddress;
}

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
    return GetLastError();
  }

  error = setBlocking(_isBlocking);

  if (ESB_SUCCESS != error) {
    close();

    return error;
  }

#if defined HAVE_CONNECT && defined HAVE_STRUCT_SOCKADDR

  if (SOCKET_ERROR == ::connect(_sockFd, (sockaddr *)_peerAddress.getAddress(),
                                sizeof(SocketAddress::Address))) {
    error = GetLastError();

    if (false == _isBlocking) {
#if defined UNIX_NONBLOCKING_CONNECT_ERROR
      if (ESB_INPROGRESS == error) {
        return ESB_SUCCESS;
      }
#elif defined WIN32_NONBLOCKING_CONNECT_ERROR
      if (ESB_AGAIN == error) {
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

  _isConnected = true;

  return ESB_SUCCESS;
}

void ConnectedTCPSocket::close() {
  TCPSocket::close();

  _isConnected = false;
}

bool ConnectedTCPSocket::isConnected() {
  if (INVALID_SOCKET == _sockFd) {
    return false;
  }

  if (_isConnected) {
    return true;
  }

#ifdef HAVE_SOCKLEN_T
  socklen_t addressSize;
#else
  int addressSize;
#endif

#if defined HAVE_GETPEERNAME && defined HAVE_STRUCT_SOCKADDR
  SocketAddress::Address address;
  addressSize = sizeof(SocketAddress::Address);

  if (SOCKET_ERROR !=
      getpeername(_sockFd, (sockaddr *)&address, &addressSize)) {
    _isConnected = true;
  }

#else
#error "getpeername or equivalent is required"
#endif

  return _isConnected;
}

bool ConnectedTCPSocket::isClient() const {
  return 0 != _listenerAddress.getPort();
}

SSize ConnectedTCPSocket::receive(char *buffer, Size bufferSize) {
#if defined HAVE_RECV
  return recv(_sockFd, buffer, bufferSize, 0);
#else
#error "recv or equivalent is required."
#endif
}

SSize ConnectedTCPSocket::receive(Buffer *buffer) {
  SSize size = receive((char *)buffer->getBuffer() + buffer->getWritePosition(),
                       buffer->getWritable());

  if (0 >= size) {
    return size;
  }

  buffer->setWritePosition(buffer->getWritePosition() + size);

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
  Size size = send((char *)buffer->getBuffer() + buffer->getReadPosition(),
                   buffer->getReadable());

  if (0 >= size) {
    return size;
  }

  buffer->setReadPosition(buffer->getReadPosition() + size);
  buffer->compact();

  return size;
}

int ConnectedTCPSocket::getBytesReadable() { return GetBytesReadable(_sockFd); }

int ConnectedTCPSocket::GetBytesReadable(SOCKET socketDescriptor) {
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

}  // namespace ESB
