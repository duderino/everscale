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
  _listeningAddress.setTransport(SocketAddress::TCP);
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
    return GetLastError();
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

    return GetLastError();
  }
#else
#error "setsockopt or equivalent is required"
#endif

#if defined HAVE_BIND && defined HAVE_STRUCT_SOCKADDR
  if (SOCKET_ERROR == ::bind(_sockFd,
                             (sockaddr *)_listeningAddress.getAddress(),
                             sizeof(SocketAddress::Address))) {
    close();

    return GetLastError();
  }
#else
#error "bind and sockaddr or equivalent is required."
#endif

  return ESB_SUCCESS;
}

Error ListeningTCPSocket::listen() {
  if (INVALID_SOCKET == _sockFd) {
    assert(0 == "Attempted to listen on an invalid socket.");

    return ESB_INVALID_STATE;
  }

#ifdef HAVE_LISTEN

  if (SOCKET_ERROR == ::listen(_sockFd, _backlog)) {
    return GetLastError();
  }

  return ESB_SUCCESS;

#else
#error "listen or equivalent is required."
#endif
}

Error ListeningTCPSocket::accept(AcceptData *data) {
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

  data->_sockFd = ::accept(_sockFd, (sockaddr *)data->_peerAddress.getAddress(),
                           &addressSize);

#else
#error "accept and sockaddr or equivalent is required."
#endif

  if (INVALID_SOCKET == data->_sockFd) {
    return GetLastError();
  }

  if (false == _isBlocking) {
    Error error = TCPSocket::SetBlocking(data->_sockFd, _isBlocking);

    if (ESB_SUCCESS != error) {
      TCPSocket::Close(data->_sockFd);
      data->_sockFd = -1;

      return error;
    }
  }

  data->_listeningAddress = _listeningAddress;
  data->_isBlocking = _isBlocking;

  return ESB_SUCCESS;
}

const SocketAddress &ListeningTCPSocket::getListeningAddress() const {
  return _listeningAddress;
}

}  // namespace ESB