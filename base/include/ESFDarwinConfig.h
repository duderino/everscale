/** @file ESFDarwinConfig.h
 *  @brief A collection of defines that describe the system calls and library
 *      routines available on Darwin (Mac OS X).
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_DARWIN_CONFIG_H
#define ESF_DARWIN_CONFIG_H

#if BYTE_ORDER == BIG_ENDIAN
#define ESF_BIG_ENDIAN
#elif BYTE_ORDER == LITTLE_ENDIAN
#define ESF_LITTLE_ENDIAN
#else
#error "cannot determine byte order"
#endif

#define ESF_32BIT

#define SIZEOF_CHAR 8
#define SIZEOF_UNSIGNED_CHAR 8
#define SIZEOF_SHORT 16
#define SIZEOF_UNSIGNED_SHORT 16
#define SIZEOF_INT 32
#define SIZEOF_UNSIGNED_INT 32
#define SIZEOF_LONG 32
#define SIZEOF_UNSIGNED_LONG 32
#define SIZEOF_LONG_LONG 64
#define SIZEOF_UNSIGNED_LONG_LONG 64

#define WCHAR_T_IS_UTF32

#define HAVE_SYS_TYPES_H
#define HAVE_OFF_T
#define HAVE_SIZE_T
#define HAVE_SSIZE_T

#define HAVE_PTHREAD_H
#define THREAD_ID_FORMAT "%p"
#define HAVE_PTHREAD_T
#define HAVE_PTHREAD_MUTEX_T
#define HAVE_PTHREAD_MUTEX_INIT
#define HAVE_PTHREAD_MUTEX_DESTROY
#define HAVE_PTHREAD_MUTEX_LOCK
#define HAVE_PTHREAD_MUTEX_TRYLOCK
#define HAVE_PTHREAD_MUTEX_UNLOCK
#define HAVE_PTHREAD_RWLOCK_T
#define HAVE_PTHREAD_RWLOCK_INIT
#define HAVE_PTHREAD_RWLOCK_DESTROY
#define HAVE_PTHREAD_RWLOCK_RDLOCK
#define HAVE_PTHREAD_RWLOCK_WRLOCK
#define HAVE_PTHREAD_RWLOCK_TRYRDLOCK
#define HAVE_PTHREAD_RWLOCK_TRYWRLOCK
#define HAVE_PTHREAD_RWLOCK_UNLOCK
#define HAVE_PTHREAD_COND_T
#define HAVE_PTHREAD_COND_INIT
#define HAVE_PTHREAD_COND_DESTROY
#define HAVE_PTHREAD_COND_SIGNAL
#define HAVE_PTHREAD_COND_BROADCAST
#define HAVE_PTHREAD_COND_WAIT
#define HAVE_PTHREAD_CREATE
#define HAVE_PTHREAD_JOIN
#define HAVE_PTHREAD_SELF

#define HAVE_SCHED_H
#define HAVE_SCHED_YIELD

#define HAVE_SIGNAL_H
#define HAVE_SIGACTION
#define HAVE_SIGACTION_T

#define HAVE_STDIO_H
#define ALLOW_CONSOLE_LOGGING
#define HAVE_VFPRINTF
#define HAVE_SNPRINTF

#define HAVE_STDARG_H
#define HAVE_VA_LIST_T
#define HAVE_VA_START
#define HAVE_VA_END

#define HAVE_ASSERT_H
#define HAVE_DECL_ASSERT

#define HAVE_STDLIB_H
#define HAVE_MALLOC
#define HAVE_FREE
#define HAVE_ABORT
#define HAVE_RAND_R
#define HAVE_DECL_RAND_MAX

#define HAVE_NETINET_IN_H
#define HAVE_STRUCT_SOCKADDR_IN
#define HAVE_DECL_INADDR_ANY
#define HAVE_HTONL
#define HAVE_HTONS
#define HAVE_NTOHL
#define HAVE_NTOHS

#define HAVE_NETDB_H
#define HAVE_HOSTENT_T
#define HAVE_GETHOSTBYNAME_DARWIN

#define HAVE_ARPA_INET_H
#define HAVE_INET_PTON
#define HAVE_INET_NTOP

#define HAVE_STRING_H
#define HAVE_MEMSET
#define HAVE_MEMCPY
#define HAVE_STRERROR
#define HAVE_STRNCPY
#define HAVE_STRLEN
#define HAVE_STRCAT

#define HAVE_UNISTD_H
#define HAVE_GETHOSTNAME
#define HAVE_CLOSE

#define HAVE_SYS_PARAM_H
#define HAVE_MAXHOSTNAMELEN_D

#define HAVE_SYS_SOCKET_H
#define HAVE_SOCKLEN_T
#define HAVE_STRUCT_SOCKADDR
#define HAVE_SOCKET
#define HAVE_BIND
#define HAVE_LISTEN
#define HAVE_ACCEPT
#define HAVE_CONNECT
#define UNIX_NONBLOCKING_CONNECT_ERROR
#define HAVE_SEND
#define HAVE_RECV
#define HAVE_GETPEERNAME
#define HAVE_SETSOCKOPT
#define HAVE_GETSOCKOPT

#define HAVE_SYS_IOCTL_H
#define HAVE_IOCTL
#define USE_IOCTL_FOR_NONBLOCK

#define HAVE_SYS_TIME_H
#define HAVE_STRUCT_TIMEVAL
#define HAVE_GETTIMEOFDAY

#define HAVE_TIME_H
#define HAVE_TIMESPEC_T
#define HAVE_NANOSLEEP

#define HAVE_SYS_SELECT_H
#define UNIX_SELECT_SEMANTICS
#define HAVE_SELECT

#define HAVE_ERRNO_H
#define HAVE_ERRNO_T
#define HAVE_ERRNO
#define EAGAIN_AND_EWOULDBLOCK_IDENTICAL

#endif /* ! ESF_DARWIN_CONFIG_H */
