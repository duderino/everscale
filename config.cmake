cmake_minimum_required(VERSION 3.5)

# See https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html

include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(TestBigEndian)

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
check_type_size("off_t" HAVE_OFF_T)
check_type_size("size_t" HAVE_SIZE_T)
check_type_size("ssize_t" HAVE_SSIZE_T)

# TODO do this with a little inline compilation define THREAD_ID_FORMAT "%ld"

check_include_file("pthread.h" HAVE_PTHREAD_H)
set(CMAKE_REQUIRED_LIBRARIES pthread)
check_type_size("pthread_t" HAVE_PTHREAD_T)
check_type_size("pthread_mutex_t" HAVE_PTHREAD_MUTEX_T)
check_symbol_exists("pthread_mutex_init" "pthread.h" HAVE_PTHREAD_MUTEX_INIT)
check_symbol_exists("pthread_mutex_destroy" "pthread.h" HAVE_PTHREAD_MUTEX_DESTROY)
check_symbol_exists("pthread_mutex_lock" "pthread.h" HAVE_PTHREAD_MUTEX_LOCK)
check_symbol_exists("pthread_mutex_trylock" "pthread.h" HAVE_PTHREAD_MUTEX_TRYLOCK)
check_symbol_exists("pthread_mutex_unlock" "pthread.h" HAVE_PTHREAD_MUTEX_UNLOCK)
check_type_size("pthread_rwlock_t" HAVE_PTHREAD_RWLOCK_T)
check_symbol_exists("pthread_rwlock_init" "pthread.h" HAVE_PTHREAD_RWLOCK_INIT)
check_symbol_exists("pthread_rwlock_destroy" "pthread.h" HAVE_PTHREAD_RWLOCK_DESTROY)
check_symbol_exists("pthread_rwlock_rdlock" "pthread.h" HAVE_PTHREAD_RWLOCK_RDLOCK)
check_symbol_exists("pthread_rwlock_wrlock" "pthread.h" HAVE_PTHREAD_RWLOCK_WRLOCK)
check_symbol_exists("pthread_rwlock_tryrdlock" "pthread.h" HAVE_PTHREAD_RWLOCK_TRYRDLOCK)
check_symbol_exists("pthread_rwlock_trywrlock" "pthread.h" HAVE_PTHREAD_RWLOCK_TRYWRLOCK)
check_symbol_exists("pthread_rwlock_unlock" "pthread.h" HAVE_PTHREAD_RWLOCK_UNLOCK)
check_type_size("pthread_cond_t" HAVE_PTHREAD_COND_T)
check_symbol_exists("pthread_cond_init" "pthread.h" HAVE_PTHREAD_COND_INIT)
check_symbol_exists("pthread_cond_destroy" "pthread.h" HAVE_PTHREAD_COND_DESTROY)
check_symbol_exists("pthread_cond_signal" "pthread.h" HAVE_PTHREAD_COND_SIGNAL)
check_symbol_exists("pthread_cond_wait" "pthread.h" HAVE_PTHREAD_COND_WAIT)
check_symbol_exists("pthread_cond_broadcast" "pthread.h" HAVE_PTHREAD_COND_BROADCAST)
check_symbol_exists("pthread_create" "pthread.h" HAVE_PTHREAD_CREATE)
check_symbol_exists("pthread_join" "pthread.h" HAVE_PTHREAD_JOIN)
check_symbol_exists("pthread_self" "pthread.h" HAVE_PTHREAD_SELF)

configure_file(config.h.in config.h @ONLY)
