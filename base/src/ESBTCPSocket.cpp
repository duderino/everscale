#ifndef ESB_TCP_SOCKET_H
#include <ESBTCPSocket.h>
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

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#if defined HAVE_SYS_IOCTL_H && defined USE_IOCTL_FOR_NONBLOCK
#include <sys/ioctl.h>
#endif

#if defined HAVE_FCNTL_H && defined USE_FCNTL_FOR_NONBLOCK
#include <fcntl.h>
#endif

namespace ESB {

TCPSocket::TCPSocket() : _isBlocking(false), _sockFd(INVALID_SOCKET) {}

TCPSocket::TCPSocket(bool isBlocking)
    : _isBlocking(isBlocking), _sockFd(INVALID_SOCKET) {}

TCPSocket::TCPSocket(AcceptData *acceptData)
    : _isBlocking(acceptData->_isBlocking), _sockFd(acceptData->_sockFd) {}

TCPSocket::~TCPSocket() { close(); }

Error TCPSocket::reset(AcceptData *acceptData) {
  if (!acceptData) {
    return ESB_NULL_POINTER;
  }

  close();

  _isBlocking = acceptData->_isBlocking;
  _sockFd = acceptData->_sockFd;

  return ESB_SUCCESS;
}

void TCPSocket::close() {
  Close(_sockFd);
  _sockFd = INVALID_SOCKET;
}

void TCPSocket::Close(SOCKET socket) {
  if (INVALID_SOCKET == socket) {
    return;
  }

#if defined HAVE_CLOSE

  ::close(socket);

#elif defined HAVE_CLOSESOCKET

  closesocket(socket);

#else
#error "close or equivalent is required for sockets"
#endif
}

Error TCPSocket::setBlocking(bool isBlocking) {
#if defined HAVE_FCNTL && defined USE_FCNTL_FOR_NONBLOCK

  int value = fcntl(_sockFd, F_GETFL, 0);

  if (SOCKET_ERROR == value) {
    return GetLastError();
  }

  if (((false == isBlocking) && (O_NONBLOCK & value)) ||
      ((true == isBlocking) && (!(O_NONBLOCK & value)))) {
    return ESB_SUCCESS;
  }

  if (true == isBlocking) {
    value |= O_NONBLOCK;
  } else {
    value &= ~O_NONBLOCK;
  }

  if (SOCKET_ERROR == fcntl(_sockFd, F_SETFL, value)) {
    return GetLastError();
  }

#elif defined HAVE_IOCTL && defined USE_IOCTL_FOR_NONBLOCK

  int value = (false == isBlocking);

  if (SOCKET_ERROR == ioctl(_sockFd, FIONBIO, &value)) {
    return GetLastError();
  }

#elif defined HAVE_IOCTLSOCKET

  unsigned long value = (false == isBlocking);

  if (SOCKET_ERROR == ioctlsocket(_sockFd, FIONBIO, &value)) {
    return GetLastError();
  }

#else
#error "Method to set socket to blocking/non-blocking is required."
#endif

  _isBlocking = isBlocking;

  return ESB_SUCCESS;
}

Error TCPSocket::SetBlocking(SOCKET sockFd, bool isBlocking) {
#if defined HAVE_FCNTL && defined USE_FCNTL_FOR_NONBLOCK

  int value = fcntl(sockFd, F_GETFL, 0);

  if (SOCKET_ERROR == value) {
    return GetLastError();
  }

  if (true == isBlocking) {
    value |= O_NONBLOCK;
  } else {
    value &= ~O_NONBLOCK;
  }

  if (SOCKET_ERROR == fcntl(sockFd, F_SETFL, value)) {
    return GetLastError();
  }

#elif defined HAVE_IOCTL && defined USE_IOCTL_FOR_NONBLOCK

  int value = (false == isBlocking);

  if (SOCKET_ERROR == ioctl(sockFd, FIONBIO, &value)) {
    return GetLastError();
  }

#elif defined HAVE_IOCTLSOCKET

  unsigned long value = (false == isBlocking);

  if (SOCKET_ERROR == ioctlsocket(sockFd, FIONBIO, &value)) {
    return GetLastError();
  }

#else
#error "Method to set socket to blocking/non-blocking is required."
#endif

  return ESB_SUCCESS;
}

Error TCPSocket::GetLastSocketError(SOCKET socket) {
  if (0 > socket) {
    return ESB_INVALID_ARGUMENT;
  }

  int error = 0;

#if defined HAVE_SOCKLEN_T
  socklen_t socketLength = sizeof(Error);
#else
  int socketLength = sizeof(Error);
#endif

#if defined HAVE_GETSOCKOPT

  if (SOCKET_ERROR ==
      getsockopt(socket, SOL_SOCKET, SO_ERROR, (char *)&error, &socketLength)) {
    return GetLastError();
  }

#else
#error "getsockopt or equivalent is required"
#endif

  return ConvertError(error);
}

}  // namespace ESB
