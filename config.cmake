cmake_minimum_required(VERSION 3.5)

# See https://cmake.org/cmake/help/latest/manual/cmake-modules.7.html

include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(TestBigEndian)
include(CheckStructHasMember)
include(CheckCXXSymbolExists)

test_big_endian(IS_BIG_ENDIAN)
if (IS_BIG_ENDIAN)
    set(ESB_BIG_ENDIAN 1)
else ()
    set(ESB_LITTLE_ENDIAN 1)
endif ()

# __WORDSIZE is a builtin.  Any header file could have been used here
check_symbol_exists("__WORDSIZE" "stdio.h" HAVE_WORDSIZE)

check_type_size("char" SIZEOF_CHAR_BYTES)
math(EXPR SIZEOF_CHAR "${SIZEOF_CHAR_BYTES} * 8")
check_type_size("short" SIZEOF_SHORT_BYTES)
math(EXPR SIZEOF_SHORT "${SIZEOF_SHORT_BYTES} * 8")
check_type_size("int" SIZEOF_INT_BYTES)
math(EXPR SIZEOF_INT "${SIZEOF_INT_BYTES} * 8")
check_type_size("long" SIZEOF_LONG_BYTES)
math(EXPR SIZEOF_LONG "${SIZEOF_LONG_BYTES} * 8")
check_type_size("long long" SIZEOF_LONG_LONG_BYTES)
math(EXPR SIZEOF_LONG_LONG "${SIZEOF_LONG_LONG_BYTES} * 8")

check_cxx_source_compiles("
int main () {
  int counter = 1;
  unsigned char c = 0;
  __asm__ __volatile__(\" lock; decl %0; sete %1\"
                       : \"=m\"(counter), \"=qm\"(c)
                       : \"m\"(counter)
                       : \"memory\");
  return 0 == counter && 1 == c;
}" HAVE_X86_ASM)

check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_type_size("off_t" HAVE_OFF_T)
check_type_size("size_t" HAVE_SIZE_T)
check_type_size("ssize_t" HAVE_SSIZE_T)

check_include_file("pthread.h" HAVE_PTHREAD_H)
set(CMAKE_REQUIRED_LIBRARIES pthread)
check_type_size("pthread_t" HAVE_PTHREAD_T)
math(EXPR SIZEOF_PTHREAD_T "${HAVE_PTHREAD_T} * 8")
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

check_include_file("semaphore.h" HAVE_SEMAPHORE_H)
check_cxx_source_compiles("
#include <semaphore.h>
int main () {
        sem_t sem;
}" HAVE_SEM_T)
check_symbol_exists("sem_init" "semaphore.h" HAVE_SEM_INIT)
check_symbol_exists("sem_wait" "semaphore.h" HAVE_SEM_WAIT)
check_symbol_exists("sem_trywait" "semaphore.h" HAVE_SEM_TRYWAIT)
check_symbol_exists("sem_post" "semaphore.h" HAVE_SEM_POST)
check_symbol_exists("sem_destroy" "semaphore.h" HAVE_SEM_DESTROY)

check_include_file("sched.h" HAVE_SCHED_H)
check_symbol_exists("sched_yield" "sched.h" HAVE_SCHED_YIELD)

check_include_file("ucontext.h" HAVE_UCONTEXT_H)
check_cxx_source_compiles("
#include <ucontext.h>
int main () {
        ucontext_t *uc;
}" HAVE_UCONTEXT_T)

check_include_file("signal.h" HAVE_SIGNAL_H)
check_symbol_exists(sigaction "signal.h" HAVE_SIGACTION)
check_struct_has_member("struct sigaction" sa_handler "signal.h" HAVE_STRUCT_SIGACTION)
check_cxx_source_compiles("
#include <signal.h>
int main () {
        sigignore(SIGTERM);
}" HAVE_SIGIGNORE)

check_include_file("stdio.h" HAVE_STDIO_H)
check_symbol_exists(vfprintf "stdio.h" HAVE_VFPRINTF)
check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
check_symbol_exists(fflush "stdio.h" HAVE_FFLUSH)
set(HAVE_FILE_T 1) # TODO detect with a test program

check_include_file("execinfo.h" HAVE_EXECINFO_H)
check_symbol_exists(backtrace "execinfo.h" HAVE_BACKTRACE)
check_symbol_exists(backtrace_symbols "execinfo.h" HAVE_BACKTRACE_SYMBOLS)

set(OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
set(CMAKE_REQUIRED_LIBRARIES -ldl)
check_cxx_source_compiles("
#include <cxxabi.h>
#include <dlfcn.h>
int main () {
        void **buffer;
        Dl_info info;
        dladdr(buffer[0], &info);
        int status;
        abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
}" HAVE_DLADDR_AND_CXA_DEMANGLE)
set(CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})
set(HAVE_DLFCN_H ${HAVE_DLADDR_AND_CXA_DEMANGLE})
set(HAVE_DLADDR ${HAVE_DLADDR_AND_CXA_DEMANGLE})
set(HAVE_CXXABI_H ${HAVE_DLADDR_AND_CXA_DEMANGLE})
set(HAVE_ABI_CXA_DEMANGLE ${HAVE_DLADDR_AND_CXA_DEMANGLE})

check_include_file("stdarg.h" HAVE_STDARG_H)
check_symbol_exists(va_start "stdarg.h" HAVE_VA_START)
check_symbol_exists(va_end "stdarg.h" HAVE_VA_END)

check_include_file("assert.h" HAVE_ASSERT_H)
check_symbol_exists(assert "assert.h" HAVE_ASSERT)

check_include_file("stdlib.h" HAVE_STDLIB_H)
check_symbol_exists(malloc "stdlib.h" HAVE_MALLOC)
check_symbol_exists(free "stdlib.h" HAVE_FREE)
check_symbol_exists(abort "stdlib.h" HAVE_ABORT)
check_symbol_exists(rand_r "stdlib.h" HAVE_RAND_R)
check_symbol_exists(RAND_MAX "stdlib.h" HAVE_RAND_MAX)

check_include_file("netinet/in.h" HAVE_NETINET_IN_H)
check_struct_has_member("struct sockaddr_in" sin_family "netinet/in.h" HAVE_STRUCT_SOCKADDR_IN)
check_symbol_exists(INADDR_ANY "netinet/in.h" HAVE_INADDR_ANY)
check_symbol_exists(htonl "netinet/in.h" HAVE_HTONL)
check_symbol_exists(htons "netinet/in.h" HAVE_HTONS)
check_symbol_exists(ntohl "netinet/in.h" HAVE_NTOHL)
check_symbol_exists(ntohs "netinet/in.h" HAVE_NTOHS)

check_include_file("netinet/tcp.h" HAVE_NETINET_TCP_H)

check_include_file("netdb.h" HAVE_NETDB_H)
check_struct_has_member("struct hostent" h_name "netdb.h" HAVE_HOSTENT_T)
check_symbol_exists(gethostbyname_r "netdb.h" HAVE_GETHOSTBYNAME_R_LINUX) # TODO write a test program to detect the Linux variant of gethostbyname_r

check_include_file("getopt.h" HAVE_GETOPT_H)
check_symbol_exists(getopt "getopt.h" HAVE_GETOPT)
check_symbol_exists(getopt_long "getopt.h" HAVE_GETOPT_LONG)

check_include_file("arpa/inet.h" HAVE_ARPA_INET_H)
check_symbol_exists(inet_pton "arpa/inet.h" HAVE_INET_PTON)
check_symbol_exists(inet_ntop "arpa/inet.h" HAVE_INET_NTOP)
check_symbol_exists(INET6_ADDRSTRLEN "arpa/inet.h" HAVE_INET6_ADDRSTRLEN)

check_include_file("string.h" HAVE_STRING_H)
check_symbol_exists(memset "string.h" HAVE_MEMSET)
check_symbol_exists(memcpy "string.h" HAVE_MEMCPY)
check_symbol_exists(strerror_r "string.h" HAVE_STRERROR_R)
check_symbol_exists(strncpy "string.h" HAVE_STRNCPY)
check_symbol_exists(strlen "string.h" HAVE_STRLEN)
check_symbol_exists(strcat "string.h" HAVE_STRCAT)
check_symbol_exists(memmove "string.h" HAVE_MEMMOVE)

check_include_file("unistd.h" HAVE_UNISTD_H)
check_symbol_exists(gethostname "unistd.h" HAVE_GETHOSTNAME)
check_symbol_exists(close "unistd.h" HAVE_CLOSE)
check_symbol_exists(write "unistd.h" HAVE_WRITE)
check_symbol_exists(read "unistd.h" HAVE_READ)
check_symbol_exists(dup "unistd.h" HAVE_DUP)
check_symbol_exists(usleep "unistd.h" HAVE_USLEEP)
check_cxx_source_compiles("
#include <unistd.h>
int main () {
  long sz = sysconf(_SC_PAGESIZE);
  return 0 < sz;
}" HAVE_SC_PAGESIZE)
set(HAVE_SYSCONF ${HAVE_SC_PAGESIZE})
check_cxx_source_compiles("
#include <unistd.h>
int main () {
  long sz = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
  return 0 < sz;
}" HAVE_SC_LEVEL1_DCACHE_LINESIZE)

if (HAVE_SC_PAGESIZE)
    execute_process(COMMAND getconf PAGE_SIZE OUTPUT_VARIABLE ESB_PAGE_SIZE OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

if (HAVE_SC_LEVEL1_DCACHE_LINESIZE)
    execute_process(COMMAND getconf LEVEL1_DCACHE_LINESIZE OUTPUT_VARIABLE ESB_CACHE_LINE_SIZE OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

check_include_file("sys/param.h" HAVE_SYS_PARAM_H)
# use 255 across all platforms. check_symbol_exists(MAXHOSTNAMELEN "sys/param.h" HAVE_MAXHOSTNAMELEN)

check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)

check_include_file("sys/socket.h" HAVE_SYS_SOCKET_H)
set(UNIX_NONBLOCKING_CONNECT_ERROR 1) # TODO detect this behavior with a test program
check_struct_has_member("struct sockaddr" sa_family "sys/socket.h" HAVE_STRUCT_SOCKADDR)
set(HAVE_SOCKLEN_T 1) # TODO detect presence with a test program
check_symbol_exists(socket "sys/socket.h" HAVE_SOCKET)
check_symbol_exists(bind "sys/socket.h" HAVE_BIND)
check_symbol_exists(listen "sys/socket.h" HAVE_LISTEN)
check_symbol_exists(getsockname "sys/socket.h" HAVE_GETSOCKNAME)
check_symbol_exists(accept "sys/socket.h" HAVE_ACCEPT)
check_symbol_exists(connect "sys/socket.h" HAVE_CONNECT)
check_symbol_exists(send "sys/socket.h" HAVE_SEND)
check_symbol_exists(recv "sys/socket.h" HAVE_RECV)
check_symbol_exists(getpeername "sys/socket.h" HAVE_GETPEERNAME)
check_symbol_exists(setsockopt "sys/socket.h" HAVE_SETSOCKOPT)
check_symbol_exists(getsockopt "sys/socket.h" HAVE_GETSOCKOPT)
check_cxx_source_compiles("
#include <sys/socket.h>
int main () {
  int sockFd = -1;
  int optval = 1;
  return setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}" HAVE_SO_REUSEPORT)

check_include_file("sys/ioctl.h" HAVE_SYS_IOCTL_H)
check_symbol_exists(ioctl "sys/ioctl.h" HAVE_IOCTL)
set(USE_IOCTL_FOR_NONBLOCK 1) #TODO make this decision with a test program

check_include_file("sys/syscall.h" HAVE_SYS_SYSCALL_H)
check_cxx_source_compiles("
#include <unistd.h>
#include <sys/syscall.h>
int main () {
  pid_t tid = syscall(SYS_gettid);
  return 0 < tid;
}" HAVE_GETTID)
set(HAVE_SYSCALL ${HAVE_GETTID})

check_include_file("sys/time.h" HAVE_SYS_TIME_H)
check_struct_has_member("struct timeval" tv_usec "sys/time.h" HAVE_STRUCT_TIMEVAL)
check_symbol_exists(gettimeofday "sys/time.h" HAVE_GETTIMEOFDAY)

check_include_file("time.h" HAVE_TIME_H)
check_struct_has_member("struct timespec" tv_nsec "time.h" HAVE_TIMESPEC_T)
check_symbol_exists(nanosleep "time.h" HAVE_NANOSLEEP)
check_type_size(time_t HAVE_TIME_T)
check_symbol_exists(time "time.h" HAVE_TIME)

check_include_file("sys/select.h" HAVE_SYS_SELECT_H)
set(UNIX_SELECT_SEMANTICS 1) #TODO make this decision with a test program
check_symbol_exists(select "sys/select.h" HAVE_SELECT)

check_include_file("errno.h" HAVE_ERRNO_H)
check_symbol_exists(errno "errno.h" HAVE_ERRNO)
set(EAGAIN_AND_EWOULDBLOCK_IDENTICAL 1) #TODO make this decision with a test program

check_include_file("sys/epoll.h" HAVE_SYS_EPOLL_H)
check_struct_has_member("struct epoll_event" events "sys/epoll.h" HAVE_STRUCT_EPOLL_EVENT)
check_symbol_exists(epoll_create "sys/epoll.h" HAVE_EPOLL_CREATE)
check_symbol_exists(epoll_ctl "sys/epoll.h" HAVE_EPOLL_CTL)
check_symbol_exists(epoll_wait "sys/epoll.h" HAVE_EPOLL_WAIT)

check_include_file("sys/resource.h" HAVE_SYS_RESOURCE_H)
check_struct_has_member("struct rlimit" rlim_max "sys/resource.h" HAVE_STRUCT_RLIMIT)
check_symbol_exists(getrlimit "sys/resource.h" HAVE_GETRLIMIT)
check_symbol_exists(setrlimit "sys/resource.h" HAVE_SETRLIMIT)

check_include_file("sys/stat.h" HAVE_SYS_STAT_H)

check_include_file("fcntl.h" HAVE_FCNTL_H)

check_include_file("sys/mman.h" HAVE_SYS_MMAN_H)
check_symbol_exists(mmap "sys/mman.h" HAVE_MMAP)
check_symbol_exists(munmap "sys/mman.h" HAVE_MUNMAP)

check_include_file_cxx("atomic" HAVE_ATOMIC_H)
check_cxx_source_compiles("#include <atomic>
int main () {
  std::atomic<int> atomic1{0};
  std::atomic<int> atomic2{0};
  atomic1++; ++atomic2;
  return atomic1 == atomic2;
}" HAVE_ATOMIC_T)

check_include_file("sys/eventfd.h" HAVE_SYS_EVENTFD_H)
check_symbol_exists(eventfd "sys/eventfd.h" HAVE_EVENTFD)

check_cxx_source_compiles("
#define likely(expr) __builtin_expect(!!(expr),1)
#define unlikely(expr) __builtin_expect(!!(expr),0)
int main () {
    if (likely(true)) {
        return 0;
    }
    if (unlikely(false)) {
        return 1;
    }
    return 0;
}" HAVE_BUILTIN_EXPECT)

configure_file(config.h.in base/include/ESBConfig.h @ONLY)
configure_file(config.h.in unit-tf/include/ESTFConfig.h @ONLY)
