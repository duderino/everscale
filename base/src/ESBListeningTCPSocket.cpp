#ifndef ESB_LISTENING_TCP_SOCKET_H
#include <ESBListeningTCPSocket.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined HAVE_SYS_IOCTL_H && defined USE_IOCTL_FOR_NONBLOCK
#include <sys/ioctl.h>
#endif

#if defined HAVE_FCNTL_H && defined USE_FCNTL_FOR_NONBLOCK
#include <fcntl.h>
#endif

namespace ESB {

ListeningTCPSocket::ListeningTCPSocket(UInt16 port, int backlog,
                                       bool isBlocking)
    : TCPSocket(isBlocking), _backlog(backlog), _listeningAddress() {
  _listeningAddress.setPort(port);
  _listeningAddress.setType(SocketAddress::TCP);
}

ListeningTCPSocket::ListeningTCPSocket(SocketAddress &address, int backlog,
                                       bool isBlocking)
    : TCPSocket(isBlocking), _backlog(backlog), _listeningAddress(address) {}

ListeningTCPSocket::~ListeningTCPSocket() {}

Error ListeningTCPSocket::bind() {
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

#ifdef HAVE_SETSOCKOPT
  int value = 1;

  if (SOCKET_ERROR == setsockopt(_sockFd, SOL_SOCKET, SO_REUSEADDR,
                                 (const char *)&value, sizeof(value))) {
    close();

    return LastError();
  }
#else
#error "setsockopt or equivalent is required"
#endif

#if defined HAVE_BIND && defined HAVE_STRUCT_SOCKADDR
  if (SOCKET_ERROR == ::bind(_sockFd,
                             (sockaddr *)_listeningAddress.primitiveAddress(),
                             sizeof(SocketAddress::Address))) {
    close();

    return LastError();
  }
#else
#error "bind and sockaddr or equivalent is required."
#endif

  if (0 != _listeningAddress.port()) {
    return ESB_SUCCESS;
  }

  // If we bound to an ephemeral port, determine the assigned port.

  SocketAddress::Address addr;

#ifdef HAVE_SOCKLEN_T
  socklen_t length = sizeof(addr);
#else
  ESB::UInt32 length = sizeof(addr);
#endif

#if defined HAVE_GETSOCKNAME
  if (SOCKET_ERROR == ::getsockname(_sockFd, (sockaddr *)&addr, &length)) {
    close();
    return LastError();
  }
  assert(length == sizeof(addr));
#else
#error "getsockname or equivalent is required."
#endif

  _listeningAddress.updatePrimitiveAddress(&addr);

  if (_logAddress[0]) {
    _listeningAddress.logAddress(_logAddress, sizeof(_logAddress), _sockFd);
  }

  return ESB_SUCCESS;
}

Error ListeningTCPSocket::listen() {
  if (INVALID_SOCKET == _sockFd) {
    assert(0 == "Attempted to listen on an invalid socket.");

    return ESB_INVALID_STATE;
  }

#ifdef HAVE_LISTEN

  if (SOCKET_ERROR == ::listen(_sockFd, _backlog)) {
    return LastError();
  }

  return ESB_SUCCESS;

#else
#error "listen or equivalent is required."
#endif
}

Error ListeningTCPSocket::accept(State *data) {
  if (!data) {
    return ESB_NULL_POINTER;
  }

#ifdef HAVE_SOCKLEN_T
  socklen_t addressSize;
#else
  int addressSize;
#endif

#if defined HAVE_ACCEPT && defined HAVE_STRUCT_SOCKADDR
  addressSize = sizeof(SocketAddress::Address);
  data->setSocketDescriptor(
      ::accept(_sockFd, (sockaddr *)data->peerAddress().primitiveAddress(),
               &addressSize));
#else
#error "accept and sockaddr or equivalent is required."
#endif

  if (INVALID_SOCKET == data->socketDescriptor()) {
    return LastError();
  }

  if (!_isBlocking) {
    Error error = TCPSocket::SetBlocking(data->socketDescriptor(), _isBlocking);

    if (ESB_SUCCESS != error) {
      TCPSocket::Close(data->socketDescriptor());
      data->setSocketDescriptor(-1);
      return error;
    }
  }

  data->setListeningAddress(_listeningAddress);
  data->setIsBlocking(_isBlocking);

  return ESB_SUCCESS;
}

const SocketAddress &ListeningTCPSocket::listeningAddress() const {
  return _listeningAddress;
}

const char *ListeningTCPSocket::logAddress() {
  if (!_logAddress[0]) {
    _listeningAddress.logAddress(_logAddress, sizeof(_logAddress), _sockFd);
  }

  return _logAddress;
}

}  // namespace ESB
