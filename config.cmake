cmake_minimum_required(VERSION 3.5)

# See https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html

include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include (TestBigEndian)

test_big_endian(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  set(ESB_BIG_ENDIAN 1)
else()
  set(ESB_LITTLE_ENDIAN 1)
endif()

# __WORDSIZE is a builtin, the header file is just to satisfy cmake
check_symbol_exists("__WORDSIZE" "stdint.h" HAVE_WORDSIZE)

check_type_size("char" SIZEOF_CHAR)
check_type_size("short" SIZEOF_SHORT)
check_type_size("int" SIZEOF_INT)
check_type_size("long" SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)
check_type_size("int" SIZEOF_INT)

# TODO #define HAVE_X86_ASM

check_include_file("assert.h" HAVE_ASSERT_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("stdio.h" HAVE_STDIO_H)
check_include_file("string.h" HAVE_STRING_H)

check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_symbol_exists("off_t" "sys/types.h" HAVE_OFF_T)
check_symbol_exists("size_t" "sys/types.h,stdlib.h,stddef.h" HAVE_SIZE_T)
check_symbol_exists("ssize_t" "sys/types.h" HAVE_SSIZE_T)


check_symbol_exists("getpagesize" "unistd.h" HAVE_GETPAGESIZE)
check_symbol_exists("mmap" "sys/mman.h" HAVE_MMAP)
check_symbol_exists("getrandom" "sys/random.h" HAVE_GETRANDOM)


configure_file(config.h.in config.h @ONLY)
