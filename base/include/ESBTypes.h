#ifndef ESB_TYPES_H
#define ESB_TYPES_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_BASETSD_H
#include <basetsd.h>
#endif

namespace ESB {

#if !(defined ESB_BIG_ENDIAN || defined ESB_LITTLE_ENDIAN)
#error "Endianness of target is unknown"
#endif

#if 8 == SIZEOF_CHAR
typedef char Int8;
#define ESB_INT8_C(c) c
#define ESB_INT8_MAX 127
#define ESB_INT8_MIN (-128)
#else
#error "8 bit signed integer required"
#endif

#if 8 == SIZEOF_UNSIGNED_CHAR
typedef unsigned char UInt8;
#define ESB_UINT8_C(c) c##U
#define ESB_UINT8_MAX 255U
#define ESB_UINT8_MIN 0U
#else
#error "8 bit unsigned integer required"
#endif

#if 16 == SIZEOF_SHORT
typedef short Int16;
#define ESB_INT16_C(c) c
#define ESB_INT16_MAX 32767
#define ESB_INT16_MIN (-32728)
#else
#error "16 bit integer required"
#endif

#if 16 == SIZEOF_UNSIGNED_SHORT
typedef unsigned short UInt16;
#define ESB_UINT16_C(c) c##U
#define ESB_UINT16_MAX 65535U
#define ESB_UINT16_MIN 0U
#else
#error "16 bit unsigned integer required"
#endif

#if 32 == SIZEOF_INT
typedef int Int32;
#define ESB_INT32_C(c) c
#define ESB_INT32_MAX 2147483647
#define ESB_INT32_MIN (-(ESB_INT32_MAX + 1))
#else
#error "32 bit integer required"
#endif

#if 32 == SIZEOF_UNSIGNED_INT
typedef unsigned int UInt32;
#define ESB_UINT32_C(c) c##U
#define ESB_UINT32_MAX 4294967295U
#define ESB_UINT32_MIN 0U
#else
#error "32 bit unsigned integer required"
#endif

#if 64 == SIZEOF_LONG
typedef long Int64;
#define ESB_INT64_C(c) c##L
#define ESB_INT64_MAX 9223372036854775807L
#define ESB_INT64_MIN (-(ESB_INT64_MAX + 1L))
#elif 64 == SIZEOF_LONG_LONG
typedef long long Int64;
#define ESB_INT64_C(c) c##LL
#define ESB_INT64_MAX 9223372036854775807LL
#define ESB_INT64_MIN (-(ESB_INT64_MAX + 1LL))
#elif 64 == SIZEOF___INT64
typedef __int64 Int64;
#define ESB_INT64_C(c) c##i64
#define ESB_INT64_MAX 9223372036854775807i64
#define ESB_INT64_MIN -(ESB_INT64_MAX + 1i64)
#else
#error "64 bit integer required"
#endif

#if 64 == SIZEOF_UNSIGNED_LONG
typedef unsigned long UInt64;
#define ESB_UINT64_C(c) c##UL
#define ESB_UINT64_MAX 18446744073709551615UL
#define ESB_UINT64_MIN 0UL
#elif 64 == SIZEOF_UNSIGNED_LONG_LONG
typedef unsigned long long UInt64;
#define ESB_UINTT64_C(c) c##ULL
#define ESB_UINT64_MAX 18446744073709551615ULL
#define ESB_UINT64_MIN 0ULL
#elif 64 == SIZEOF_UNSIGNED___INT64
typedef unsigned __int64 UInt64;
#define ESB_UINT64_C(c) c##ui64
#define ESB_UINT64_MAX 9223372036854775807ui64
#define ESB_UINT64_MIN 0ui64
#else
#error "64 bit unsigned integer required"
#endif

#ifdef ESB_32BIT
typedef Int32 Word;
#define ESB_WORD_C ESB_INT32_C
#define ESB_WORD_MAX ESB_INT32_MAX
#define ESB_WORD_MIN ESB_INT32_MIN
typedef UInt32 UWord;
#define ESB_UWORD_C ESB_UINT32_C
#define ESB_UWORD_MAX ESB_UINT32_MAX
#define ESB_UWORD_MIN ESB_UINT32_MIN
#elif defined ESB_64BIT
typedef Int64 Word;
#define ESB_WORD_C ESB_INT64_C
#define ESB_WORD_MAX ESB_INT64_MAX
#define ESB_WORD_MIN ESB_INT64_MIN
typedef UInt64 UWord;
#define ESB_UWORD_C ESB_UINT64_C
#define ESB_UWORD_MAX ESB_UINT64_MAX
#define ESB_UWORD_MIN ESB_UINT64_MIN
#else
#error "Unknown architecture"
#endif

#ifdef HAVE_SIZE_T
typedef size_t Size;
#elif defined HAVE_SIZE_T_UC
typedef SIZE_T Size;
#else
#error "Size type required"
#endif

#ifdef HAVE_SSIZE_T
typedef ssize_t SSize;
#elif defined HAVE_SSIZE_T_UC
typedef SSIZE_T SSize;
#else
#error "Signed size type required"
#endif

#define ESB_MAGIC ESB_UINT8_C(0x23)

// Note, size must be a power of two
#define ESB_ALIGN(value, size) (((value) % (size)) ? (((value) & ~((size)-1)) + (size)) : (value))
#define ESB_WORD_ALIGN(value) ESB_ALIGN(value, sizeof(ESB::Word))

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define ESB_NAME_PREFIX_SIZE 16
#define ESB_MAX_UINT16_STRING_LENGTH 5
#define ESB_MAX_UINT32_STRING_LENGTH 10
#define ESB_ADDRESS_PORT_SIZE (ESB_IPV6_PRESENTATION_SIZE + ESB_MAX_UINT16_STRING_LENGTH + 1)

#define ESB_MAX_HOSTNAME 255
#define ESB_MAX_PATH 4096
#define ESB_MAX_FILENAME 255

#define ESB_DISABLE_AUTO_COPY(CLASS) \
 private:                            \
  CLASS(const CLASS &);              \
  void operator=(const CLASS &);

typedef struct {
  unsigned char *_data;
  UInt32 _capacity;
} SizedBuffer;

}  // namespace ESB

#endif
