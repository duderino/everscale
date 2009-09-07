/** @file ESFTypes.h
 *  @brief A collection of primitive type abstractions
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

#ifndef ESF_TYPES_H
#define ESF_TYPES_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_BASETSD_H
#include <basetsd.h>
#endif

#if ! ( defined ESF_BIG_ENDIAN || defined ESF_LITTLE_ENDIAN )
#error "Endianness of target is unknown"
#endif

#if 8 == SIZEOF_CHAR
typedef char ESFInt8;
#define ESF_INT8_C(c) c
#define ESF_INT8_MAX 127
#define ESF_INT8_MIN (-128)
#else
#error "8 bit signed integer required"
#endif

#if 8 == SIZEOF_UNSIGNED_CHAR
typedef unsigned char ESFUInt8;
#define ESF_UINT8_C(c) c ## U
#define ESF_UINT8_MAX 255U
#define ESF_UINT8_MIN 0U
#else
#error "8 bit unsigned integer required"
#endif

#if 16 == SIZEOF_SHORT
typedef short ESFInt16;
#define ESF_INT16_C(c) c
#define ESF_INT16_MAX 32767
#define ESF_INT16_MIN (-32728)
#else
#error "16 bit integer required"
#endif

#if 16 == SIZEOF_UNSIGNED_SHORT
typedef unsigned short ESFUInt16;
#define ESF_UINT16_C(c) c ## U
#define ESF_UINT16_MAX 65535U
#define ESF_UINT16_MIN 0U
#else
#error "16 bit unsigned integer required"
#endif

#if 32 == SIZEOF_INT
typedef int ESFInt32;
#define ESF_INT32_C(c) c
#define ESF_INT32_MAX 2147483647
#define ESF_INT32_MIN (-(ESF_INT32_MAX+1))
#else
#error "32 bit integer required"
#endif

#if 32 == SIZEOF_UNSIGNED_INT
typedef unsigned int ESFUInt32;
#define ESF_UINT32_C(c) c ## U
#define ESF_UINT32_MAX 4294967295U
#define ESF_UINT32_MIN 0U
#else
#error "32 bit unsigned integer required"
#endif

#if 64 == SIZEOF_LONG
typedef long ESFInt64;
#define ESF_INT64_C(c) c ## L
#define ESF_INT64_MAX 9223372036854775807L
#define ESF_INT64_MIN (-(ESF_INT64_MAX+1L))
#elif 64 == SIZEOF_LONG_LONG
typedef long long ESFInt64;
#define ESF_INT64_C(c) c ## LL
#define ESF_INT64_MAX 9223372036854775807LL
#define ESF_INT64_MIN (-(ESF_INT64_MAX+1LL))
#elif 64 == SIZEOF___INT64
typedef __int64 ESFInt64;
#define ESF_INT64_C(c) c ## i64
#define ESF_INT64_MAX 9223372036854775807i64
#define ESF_INT64_MIN -(ESF_INT64_MAX+1i64)
#else
#error "64 bit integer required"
#endif

#if 64 == SIZEOF_UNSIGNED_LONG
typedef unsigned long ESFUInt64;
#define ESF_UINT64_C(c) c ## UL
#define ESF_UINT64_MAX 18446744073709551615UL
#define ESF_UINT64_MIN 0UL
#elif 64 == SIZEOF_UNSIGNED_LONG_LONG
typedef unsigned long long ESFUInt64;
#define ESF_UINTT64_C(c) c ## ULL
#define ESF_UINT64_MAX 18446744073709551615ULL
#define ESF_UINT64_MIN 0ULL
#elif 64 == SIZEOF_UNSIGNED___INT64
typedef unsigned __int64 ESFUInt64;
#define ESF_UINT64_C(c) c ## ui64
#define ESF_UINT64_MAX 9223372036854775807ui64
#define ESF_UINT64_MIN 0ui64
#else
#error "64 bit unsigned integer required"
#endif

#ifdef ESF_32BIT
typedef ESFInt32 ESFWord;
#define ESF_WORD_C ESF_INT32_C
#define ESF_WORD_MAX ESF_INT32_MAX
#define ESF_WORD_MIN ESF_INT32_MIN
typedef ESFUInt32 ESFUWord;
#define ESF_UWORD_C ESF_UINT32_C
#define ESF_UWORD_MAX ESF_UINT32_MAX
#define ESF_UWORD_MIN ESF_UINT32_MIN
#elif defined ESF_64BIT
typedef ESFInt64 ESFWord;
#define ESF_WORD_C ESF_INT64_C
#define ESF_WORD_MAX ESF_INT64_MAX
#define ESF_WORD_MIN ESF_INT64_MIN
typedef ESFUInt64 ESFUWord;
#define ESF_UWORD_C ESF_UINT64_C
#define ESF_UWORD_MAX ESF_UINT64_MAX
#define ESF_UWORD_MIN ESF_UINT64_MIN
#else
#error "Unknown architecture"
#endif

#ifdef HAVE_SIZE_T
typedef size_t ESFSize;
#elif defined HAVE_SIZE_T_UC
typedef SIZE_T ESFSize;
#else
#error "Size type required"
#endif

#ifdef HAVE_SSIZE_T
typedef ssize_t ESFSSize;
#elif defined HAVE_SSIZE_T_UC
typedef SSIZE_T ESFSSize;
#else
#error "Signed size type required"
#endif

#define ESF_MAGIC ESF_UINT8_C(0x23)

#define ESF_WORD_ALIGN(value) (((value) % sizeof(ESFWord)) ? (((value) & ~(sizeof(ESFWord) - 1)) + sizeof(ESFWord)) : (value))

#define ESF_MIN(a, b) ((a) > (b) ? (b) : (a))

#define ESF_MAX(a, b) ((a) > (b) ? (a) : (b))

#endif /* ! ESF_TYPES_H */
