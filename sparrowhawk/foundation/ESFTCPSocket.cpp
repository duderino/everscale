/** @file ESFTCPSocket.cpp
 *  @brief An abstract base class with code common to both
 *      ESFConnectedTCPSocket and ESFListeningTCPSocket instances
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_TCP_SOCKET_H
#include <ESFTCPSocket.h>
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

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif

#if defined HAVE_SYS_IOCTL_H && defined USE_IOCTL_FOR_NONBLOCK
#include <sys/ioctl.h>
#endif

#if defined HAVE_FCNTL_H && defined USE_FCNTL_FOR_NONBLOCK
#include <fcntl.h>
#endif

ESFTCPSocket::ESFTCPSocket() :
    _isBlocking(false), _sockFd(INVALID_SOCKET) {
}

ESFTCPSocket::ESFTCPSocket(bool isBlocking) :
    _isBlocking(isBlocking), _sockFd(INVALID_SOCKET) {
}

ESFTCPSocket::ESFTCPSocket(AcceptData *acceptData) :
    _isBlocking(acceptData->_isBlocking), _sockFd(acceptData->_sockFd) {
}

ESFTCPSocket::~ESFTCPSocket() {
    close();
}

ESFError ESFTCPSocket::reset(AcceptData *acceptData) {
    if (!acceptData) {
        return ESF_NULL_POINTER;
    }

    close();

    _isBlocking = acceptData->_isBlocking;
    _sockFd = acceptData->_sockFd;

    return ESF_SUCCESS;
}

void ESFTCPSocket::close() {
    Close(_sockFd);
    _sockFd = INVALID_SOCKET;
}

void ESFTCPSocket::Close(SOCKET socket) {
    if (INVALID_SOCKET == socket) {
        return;
    }

#if defined HAVE_CLOSE

    ::close(socket);

#elif defined HAVE_CLOSESOCKET

    closesocket( socket );

#else
#error "close or equivalent is required for sockets"
#endif
}

ESFError ESFTCPSocket::setBlocking(bool isBlocking) {
#if defined HAVE_FCNTL && defined USE_FCNTL_FOR_NONBLOCK

    int value = fcntl( _sockFd, F_GETFL, 0 );

    if ( SOCKET_ERROR == value )
    {
        return ESFGetLastError();
    }

    if ( ( ( false == isBlocking ) && ( O_NONBLOCK & value ) ) ||
            ( ( true == isBlocking ) && ( ! ( O_NONBLOCK & value ) ) ) )
    {
        return ESF_SUCCESS;
    }

    if ( true == isBlocking )
    {
        value |= O_NONBLOCK;
    }
    else
    {
        value &= ~O_NONBLOCK;
    }

    if ( SOCKET_ERROR == fcntl( _sockFd, F_SETFL, value ) )
    {
        return ESFGetLastError();
    }

#elif defined HAVE_IOCTL && defined USE_IOCTL_FOR_NONBLOCK

    int value = (false == isBlocking);

    if (SOCKET_ERROR == ioctl(_sockFd, FIONBIO, &value)) {
        return ESFGetLastError();
    }

#elif defined HAVE_IOCTLSOCKET

    unsigned long value = ( false == isBlocking );

    if ( SOCKET_ERROR == ioctlsocket( _sockFd, FIONBIO, &value ) )
    {
        return ESFGetLastError();
    }

#else
#error "Method to set socket to blocking/non-blocking is required."
#endif

    _isBlocking = isBlocking;

    return ESF_SUCCESS;
}

ESFError ESFTCPSocket::SetBlocking(SOCKET sockFd, bool isBlocking) {
#if defined HAVE_FCNTL && defined USE_FCNTL_FOR_NONBLOCK

    int value = fcntl( sockFd, F_GETFL, 0 );

    if ( SOCKET_ERROR == value )
    {
        return ESFGetLastError();
    }

    if ( true == isBlocking )
    {
        value |= O_NONBLOCK;
    }
    else
    {
        value &= ~O_NONBLOCK;
    }

    if ( SOCKET_ERROR == fcntl( sockFd, F_SETFL, value ) )
    {
        return ESFGetLastError();
    }

#elif defined HAVE_IOCTL && defined USE_IOCTL_FOR_NONBLOCK

    int value = (false == isBlocking);

    if (SOCKET_ERROR == ioctl(sockFd, FIONBIO, &value)) {
        return ESFGetLastError();
    }

#elif defined HAVE_IOCTLSOCKET

    unsigned long value = ( false == isBlocking );

    if ( SOCKET_ERROR == ioctlsocket( sockFd, FIONBIO, &value ) )
    {
        return ESFGetLastError();
    }

#else
#error "Method to set socket to blocking/non-blocking is required."
#endif

    return ESF_SUCCESS;
}

ESFError ESFTCPSocket::GetLastError(SOCKET socket) {
    if (0 > socket) {
        return ESF_INVALID_ARGUMENT;
    }

    int error = 0;

#if defined HAVE_SOCKLEN_T
    socklen_t socketLength = sizeof(ESFError);
#else
    int socketLength = sizeof( ESFError );
#endif

#if defined HAVE_GETSOCKOPT

    if (SOCKET_ERROR == getsockopt(socket, SOL_SOCKET, SO_ERROR, (char *) &error, &socketLength)) {
        return ESFGetLastError();
    }

#else
#error "getsockopt or equivalent is required"
#endif

    return ESFConvertError(error);
}
