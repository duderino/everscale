/** @file ESFWin32Config.h
 *  @brief A collection of defines that describe the system calls and library
 *      routines available on 32-bit Windows systems.
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

#ifndef ESF_WIN32_CONFIG_H
#define ESF_WIN32_CONFIG_H

#define ESF_LITTLE_ENDIAN

#ifdef _WIN64
#define ESF_64BIT
#elif defined _WIN32
#define ESF_32BIT
#else
#error "Unknown word size"
#endif

#define SIZEOF_CHAR 8
#define SIZEOF_UNSIGNED_CHAR 8
#define SIZEOF_SHORT 16
#define SIZEOF_UNSIGNED_SHORT 16
#define SIZEOF_INT 32
#define SIZEOF_UNSIGNED_INT 32
#define SIZEOF_LONG 32
#define SIZEOF_UNSIGNED_LONG 32
#define SIZEOF___INT64 64
#define SIZEOF_UNSIGNED___INT64 64

#define HAVE_BASETSD_H
#define HAVE_SIZE_T_UC
#define HAVE_SSIZE_T_UC

#define HAVE_ASSERT_H
#define HAVE_DECL_ASSERT

#define HAVE_WINSOCK2_H
#define HAVE_STRUCT_SOCKADDR
#define HAVE_STRUCT_SOCKADDR_IN
#define HAVE_SOCKET_T
#define HAVE_DECL_INVALID_SOCKET
#define HAVE_DECL_SOCKET_ERROR
#define HAVE_SOCKET
#define HAVE_CONNECT
#define HAVE_GETPEERNAME
#define HAVE_RECV
#define HAVE_SEND
#define HAVE_IOCTLSOCKET
#define WIN32_NONBLOCKING_CONNECT_ERROR
#define HAVE_CLOSESOCKET
#define HAVE_GETSOCKOPT
#define HAVE_HTONS
#define HAVE_HTONL
#define HAVE_NTOHS
#define HAVE_NTOHL
#define HAVE_DECL_INADDR_ANY
#define HAVE_DECL_INADDR_NONE
#define HAVE_INET_ADDR
#define HAVE_INET_NTOA
#define HAVE_SETSOCKOPT
#define HAVE_BIND
#define HAVE_LISTEN
#define HAVE_ACCEPT

#define HAVE_WINDOWS_H
#define HAVE_HANDLE
#define HAVE_FILETIME
#define HAVE_CREATE_SEMAPHORE
#define HAVE_CLOSE_HANDLE
#define HAVE_WAIT_FOR_SINGLE_OBJECT
#define HAVE_RELEASE_SEMAPHORE
#define HAVE_GET_SYSTEM_TIME_AS_FILE_TIME
#define HAVE_32_X_32_TO_64
#define HAVE_FORMAT_MESSAGE
#define HAVE_GETLASTERROR
#define HAVE_CREATEMUTEX
#define HAVE_RELEASEMUTEX
#define HAVE_WAITFORSINGLEOBJECT

#define HAVE_TCHAR_H
#define HAVE_TCHAR

#define HAVE_STRING_H
#define HAVE_MEMSET
#define HAVE_MEMCPY
#define HAVE_STRNCPY

#define HAVE_STDDEF_H

#define HAVE_STDLIB_H
#define HAVE_RAND
#define HAVE_DECL_RAND_MAX

#endif /* ! ESF_WIN32_CONFIG_H */
