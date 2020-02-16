/** @file ESFSolarisConfig.h
 *  @brief A collection of defines that describe the system calls and library
 *      routines available on Solaris.
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

#ifndef ESF_SOLARIS_CONFIG_H
#define ESF_SOLARIS_CONFIG_H

#ifdef _BIG_ENDIAN
#define ESF_BIG_ENDIAN
#elif defined _LITTLE_ENDIAN
#define ESF_LITTLE_ENDIAN
#endif

#define SIZEOF_CHAR 8
#define SIZEOF_UNSIGNED_CHAR 8
#define SIZEOF_SHORT 16
#define SIZEOF_UNSIGNED_SHORT 16
#define SIZEOF_INT 32
#define SIZEOF_UNSIGNED_INT 32
#define SIZEOF_LONG_LONG 64
#define SIZEOF_UNSIGNED_LONG_LONG 64

#ifdef _LP64
#define ESF_64BIT
#define SIZEOF_LONG 64
#define SIZEOF_UNSIGNED_LONG 64
#elif define _ILP32
#define ESF_32BIT
#define SIZEOF_LONG 32
#define SIZEOF_UNSIGNED_LONG 32
#else
#error "Cannot determine Solaris wordsize"
#endif

#define HAVE_INTTYPES_H
#define HAVE_INT8_T
#define HAVE_UINT8_T
#define HAVE_INT16_T
#define HAVE_UINT16_T
#define HAVE_INT32_T
#define HAVE_UINT32_T
#define HAVE_INT64_T
#define HAVE_UINT64_T
#define HAVE_UINTPTR_T

#define HAVE_SYS_TYPES_H
#define HAVE_OFF_T
#define HAVE_SIZE_T
#define HAVE_SSIZE_T

#define HAVE_PTHREAD_H
#define THREAD_ID_IS_LONG
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
#define HAVE_PTHREAD_COND_WAIT
#define HAVE_PTHREAD_CREATE
#define HAVE_PTHREAD_JOIN
#define HAVE_PTHREAD_SELF

#define HAVE_SCHED_H
#define HAVE_SCHED_YIELD

#define HAVE_SEMAPHORE_H
#define HAVE_SEM_T
#define HAVE_SEM_INIT
#define HAVE_SEM_DESTROY
#define HAVE_SEM_WAIT
#define HAVE_SEM_TRYWAIT
#define HAVE_SEM_POST

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
#define HAVE_RAND_R
#define HAVE_DECL_RAND_MAX

#define HAVE_NETINET_IN_H
#define HAVE_SOCKADDR_IN_T
#define HAVE_DECL_INADDR_ANY
#define HAVE_HTONL
#define HAVE_HTONS
#define HAVE_NTOHL
#define HAVE_NTOHS

#define HAVE_NETDB_H
#define HAVE_HOSTENT_T
#define HAVE_MAXHOSTNAMELEN_D
#define HAVE_GETHOSTBYNAME_R_SOLARIS

#define HAVE_ARPA_INET_H
#define HAVE_INET_ADDR
#define HAVE_INET_NTOA

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

#define HAVE_SYS_IOCTL_H
#define HAVE_SYS_FILIO_H
#define HAVE_IOCTL
#define USE_IOCTL_FOR_NONBLOCK

#define HAVE_SYS_SOCKET_H
#define HAVE_SOCKADDR_T
#define HAVE_SOCKET
#define HAVE_BIND
#define HAVE_LISTEN
#define HAVE_ACCEPT
#define HAVE_CONNECT
#define HAVE_SEND
#define HAVE_RECV
#define HAVE_GETPEERNAME
#define HAVE_SETSOCKOPT
#define HAVE_GETSOCKOPT

#define HAVE_SYS_TIME_H
#define UNIX_SELECT_SEMANTICS
#define HAVE_SELECT

#define HAVE_SYS_TIME_H
#define HAVE_TIMEVAL_T
#define HAVE_GETTIMEOFDAY

#define HAVE_SYS_SELECT_H
#define UNIX_SELECT_SEMANTICS
#define HAVE_SELECT

#define HAVE_ERRNO_H
#define HAVE_ERRNO_T
#define HAVE_ERRNO
#define EAGAIN_AND_EWOULDBLOCK_IDENTICAL

#endif /* ! ESF_SOLARIS_CONFIG_H */
