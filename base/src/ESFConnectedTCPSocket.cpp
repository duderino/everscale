/** @file ESFConnectedTCPSocket.cpp
 *  @brief A wrapper class for TCP connected sockets
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 *Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 *both BSD and Apache 2.0 licenses
 *(http://sourceforge.net/projects/sparrowhawk/).
 *
 *	$Author: blattj $
 *	$Date: 2009/05/25 21:51:08 $
 *	$Name:  $
 *	$Revision: 1.3 $
 */

#ifndef ESF_CONNECTED_TCP_SOCKET_H
#include <ESFConnectedTCPSocket.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
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

ESFConnectedTCPSocket::ESFConnectedTCPSocket()
    : ESFTCPSocket(), _isConnected(false), _listenerAddress(), _peerAddress() {}

ESFConnectedTCPSocket::ESFConnectedTCPSocket(bool isBlocking)
    : ESFTCPSocket(isBlocking),
      _isConnected(false),
      _listenerAddress(),
      _peerAddress() {}

ESFConnectedTCPSocket::ESFConnectedTCPSocket(const ESFSocketAddress &peer,
                                             bool isBlocking)
    : ESFTCPSocket(isBlocking),
      _isConnected(false),
      _listenerAddress(),
      _peerAddress(peer) {}

ESFConnectedTCPSocket::ESFConnectedTCPSocket(AcceptData *acceptData)
    : ESFTCPSocket(acceptData),
      _isConnected(true),
      _listenerAddress(acceptData->_listeningAddress),
      _peerAddress(acceptData->_peerAddress) {}

ESFConnectedTCPSocket::~ESFConnectedTCPSocket() {}

ESFError ESFConnectedTCPSocket::reset(AcceptData *acceptData) {
  if (!acceptData) {
    return ESF_NULL_POINTER;
  }

  ESFError error = ESFTCPSocket::reset(acceptData);

  if (ESF_SUCCESS != error) {
    return error;
  }

  _isConnected = true;
  _listenerAddress = acceptData->_listeningAddress;
  _peerAddress = acceptData->_peerAddress;

  return ESF_SUCCESS;
}

void ESFConnectedTCPSocket::setPeerAddress(const ESFSocketAddress &address) {
  ESF_ASSERT(INVALID_SOCKET == _sockFd);

  _peerAddress = address;
}

const ESFSocketAddress &ESFConnectedTCPSocket::getPeerAddress() const {
  return _peerAddress;
}

const ESFSocketAddress &ESFConnectedTCPSocket::getListenerAddress() const {
  return _listenerAddress;
}

ESFError ESFConnectedTCPSocket::connect() {
  ESFError error = ESF_SUCCESS;

  if (INVALID_SOCKET != _sockFd) {
    return ESF_INVALID_STATE;
  }

#ifdef HAVE_SOCKET
  _sockFd = socket(AF_INET, SOCK_STREAM, 0);
#else
#error "socket or equivalent is required"
#endif

  if (INVALID_SOCKET == _sockFd) {
    return ESFGetLastError();
  }

  error = setBlocking(_isBlocking);

  if (ESF_SUCCESS != error) {
    close();

    return error;
  }

#if defined HAVE_CONNECT && defined HAVE_STRUCT_SOCKADDR

  if (SOCKET_ERROR == ::connect(_sockFd, (sockaddr *)_peerAddress.getAddress(),
                                sizeof(ESFSocketAddress::Address))) {
    error = ESFGetLastError();

    if (false == _isBlocking) {
#if defined UNIX_NONBLOCKING_CONNECT_ERROR
      if (ESF_INPROGRESS == error) {
        return ESF_SUCCESS;
      }
#elif defined WIN32_NONBLOCKING_CONNECT_ERROR
      if (ESF_AGAIN == error) {
        return ESF_SUCCESS;
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

  return ESF_SUCCESS;
}

void ESFConnectedTCPSocket::close() {
  ESFTCPSocket::close();

  _isConnected = false;
}

bool ESFConnectedTCPSocket::isConnected() {
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
  ESFSocketAddress::Address address;
  addressSize = sizeof(ESFSocketAddress::Address);

  if (SOCKET_ERROR !=
      getpeername(_sockFd, (sockaddr *)&address, &addressSize)) {
    _isConnected = true;
  }

#else
#error "getpeername or equivalent is required"
#endif

  return _isConnected;
}

bool ESFConnectedTCPSocket::isClient() const {
  return 0 != _listenerAddress.getPort();
}

ESFSSize ESFConnectedTCPSocket::receive(char *buffer, ESFSize bufferSize) {
#if defined HAVE_RECV
  return recv(_sockFd, buffer, bufferSize, 0);
#else
#error "recv or equivalent is required."
#endif
}

ESFSSize ESFConnectedTCPSocket::receive(ESFBuffer *buffer) {
  ESFSSize size =
      receive((char *)buffer->getBuffer() + buffer->getWritePosition(),
              buffer->getWritable());

  if (0 >= size) {
    return size;
  }

  buffer->setWritePosition(buffer->getWritePosition() + size);

  return size;
}

ESFSSize ESFConnectedTCPSocket::send(const char *buffer, ESFSize bufferSize) {
#if defined HAVE_SEND
  return ::send(_sockFd, buffer, bufferSize, 0);
#else
#error "send or equivalent is required."
#endif
}

ESFSSize ESFConnectedTCPSocket::send(ESFBuffer *buffer) {
  ESFSize size = send((char *)buffer->getBuffer() + buffer->getReadPosition(),
                      buffer->getReadable());

  if (0 >= size) {
    return size;
  }

  buffer->setReadPosition(buffer->getReadPosition() + size);
  buffer->compact();

  return size;
}

int ESFConnectedTCPSocket::getBytesReadable() {
  return GetBytesReadable(_sockFd);
}

int ESFConnectedTCPSocket::GetBytesReadable(SOCKET socketDescriptor) {
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
