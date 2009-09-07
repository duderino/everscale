/** @file ESFListeningTCPSocket.cpp
 *  @brief A TCP socket bound to a port and ip address that is capable of
 *      receiving incoming connections.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *	$Author: blattj $
 *	$Date: 2009/05/25 21:51:08 $
 *	$Name:  $
 *	$Revision: 1.3 $
 */

#ifndef ESF_LISTENING_TCP_SOCKET_H
#include <ESFListeningTCPSocket.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
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

ESFListeningTCPSocket::ESFListeningTCPSocket(ESFUInt16 port, int backlog, bool isBlocking) :
    ESFTCPSocket(isBlocking), _backlog(backlog), _listeningAddress() {
    _listeningAddress.setPort(port);
    _listeningAddress.setTransport(ESFSocketAddress::TCP);
}

ESFListeningTCPSocket::ESFListeningTCPSocket(ESFSocketAddress &address, int backlog, bool isBlocking) :
    ESFTCPSocket(isBlocking), _backlog(backlog), _listeningAddress(address) {
}

ESFListeningTCPSocket::~ESFListeningTCPSocket() {
}

ESFError ESFListeningTCPSocket::bind() {
    ESFError error = ESF_SUCCESS;

    if (INVALID_SOCKET != _sockFd) {
        return ESF_INVALID_STATE;
    }

#ifdef HAVE_SOCKET
    _sockFd = socket(AF_INET,SOCK_STREAM, 0);
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

#ifdef HAVE_SETSOCKOPT
    int value = 1;

    if (SOCKET_ERROR == setsockopt(_sockFd, SOL_SOCKET, SO_REUSEADDR, (const char *) &value, sizeof(value))) {
        close();

        return ESFGetLastError();
    }
#else
#error "setsockopt or equivalent is required"
#endif

#if defined HAVE_BIND && defined HAVE_STRUCT_SOCKADDR
    if (SOCKET_ERROR == ::bind(_sockFd, (sockaddr *) _listeningAddress.getAddress(), sizeof(ESFSocketAddress::Address))) {
        close();

        return ESFGetLastError();
    }
#else
#error "bind and sockaddr or equivalent is required."
#endif

    return ESF_SUCCESS;
}

ESFError ESFListeningTCPSocket::listen() {
    if (INVALID_SOCKET == _sockFd) {
        ESF_ASSERT( 0 == "Attempted to listen on an invalid socket." );

        return ESF_INVALID_STATE;
    }

#ifdef HAVE_LISTEN

    if (SOCKET_ERROR == ::listen(_sockFd, _backlog)) {
        return ESFGetLastError();
    }

    return ESF_SUCCESS;

#else
#error "listen or equivalent is required."
#endif
}

ESFError ESFListeningTCPSocket::accept(AcceptData *data) {
    if (!data) {
        return ESF_NULL_POINTER;
    }

#ifdef HAVE_SOCKLEN_T
    socklen_t addressSize;
#else
    int addressSize;
#endif

#if defined HAVE_ACCEPT && defined HAVE_STRUCT_SOCKADDR

    addressSize = sizeof(ESFSocketAddress::Address);

    data->_sockFd = ::accept(_sockFd, (sockaddr *) data->_peerAddress.getAddress(), &addressSize);

#else
#error "accept and sockaddr or equivalent is required."
#endif

    if (INVALID_SOCKET == data->_sockFd) {
        return ESFGetLastError();
    }

    if (false == _isBlocking) {
        ESFError error = ESFTCPSocket::SetBlocking(data->_sockFd, _isBlocking);

        if (ESF_SUCCESS != error) {
            ESFTCPSocket::Close(data->_sockFd);
            data->_sockFd = -1;

            return error;
        }
    }

    data->_listeningAddress = _listeningAddress;
    data->_isBlocking = _isBlocking;

    return ESF_SUCCESS;
}

const ESFSocketAddress &
ESFListeningTCPSocket::getListeningAddress() const {
    return _listeningAddress;
}
