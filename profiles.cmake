cmake_minimum_required(VERSION 3.11)

unset(DEFAULT_BUILD_TYPE)
set(DEFAULT_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "build type" FORCE)
set(COMMON_C_FLAGS "-Wall -Werror -D_REENTRANT -D_GNU_SOURCE -Wno-error=deprecated-declarations")
set(COMMON_CXX_FLAGS "${COMMON_C_FLAGS} -fno-exceptions -fno-rtti")

if (DEFINED ENV{BUILD_TYPE})
    set(CMAKE_BUILD_TYPE $ENV{BUILD_TYPE})
    set(CMAKE_BUILD_TYPE $ENV{BUILD_TYPE} CACHE STRING "build type" FORCE)
endif ()

if (DEFINED ENV{EXTRA_CFLAGS})
    set(EXTRA_CFLAGS $ENV{EXTRA_CFLAGS} CACHE STRING "build type" FORCE)
else ()
    set(EXTRA_CFLAGS "" CACHE STRING "build type" FORCE)
endif ()

if (DEFINED ENV{EXTRA_CXXFLAGS})
    set(EXTRA_CXXFLAGS $ENV{EXTRA_CXXFLAGS} CACHE STRING "build type" FORCE)
else ()
    set(EXTRA_CXXFLAGS "" CACHE STRING "build type" FORCE)
endif ()

set(VALID_BUILD_TYPES DEFAULT ASAN TSAN DEBUG RELEASE RELWITHDEBINFO MINSIZEREL DEBUGNOPOOL RELEASENOPOOL COVERAGE)
if (NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "ENV{BUILD_TYPE} '${CMAKE_BUILD_TYPE}' not in (DEFAULT|ASAN|TSAN|DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL|DEBUGNOPOOL|RELEASENOPOOL|COVERAGE)")
else ()
    message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif ()

set(CMAKE_C_FLAGS_DEFAULT "-O3 -ggdb ${COMMON_C_FLAGS} -fno-omit-frame-pointer ${EXTRA_CFLAGS}" CACHE STRING "default C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEFAULT "-O3 -ggdb ${COMMON_CXX_FLAGS} -fno-omit-frame-pointer ${EXTRA_CXXFLAGS}" CACHE STRING "default C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEFAULT "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_DEFAULT CMAKE_C_FLAGS_DEFAULT CMAKE_EXE_LINKER_FLAGS_DEFAULT CMAKE_SHARED_LINKER_FLAGS_DEFAULT)

set(CMAKE_C_FLAGS_ASAN "-O0 -ggdb ${COMMON_C_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak ${EXTRA_CFLAGS}" CACHE STRING "asan C flags" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-O0 -ggdb ${COMMON_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak ${EXTRA_CXXFLAGS}" CACHE STRING "asan C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_ASAN "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_ASAN CMAKE_C_FLAGS_ASAN CMAKE_EXE_LINKER_FLAGS_ASAN CMAKE_SHARED_LINKER_FLAGS_ASAN)

set(CMAKE_C_FLAGS_TSAN "-O0 -ggdb ${COMMON_C_FLAGS} -fno-omit-frame-pointer -fsanitize=thread ${EXTRA_CFLAGS}" CACHE STRING "tsan C flags" FORCE)
set(CMAKE_CXX_FLAGS_TSAN "-O0 -ggdb ${COMMON_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=thread ${EXTRA_CXXFLAGS}" CACHE STRING "tsan C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_TSAN "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_TSAN CMAKE_C_FLAGS_TSAN CMAKE_EXE_LINKER_FLAGS_TSAN CMAKE_SHARED_LINKER_FLAGS_TSAN)

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb ${COMMON_C_FLAGS} -fno-omit-frame-pointer -fsanitize=leak ${EXTRA_CFLAGS}" CACHE STRING "debug C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb ${COMMON_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=leak ${EXTRA_CXXFLAGS}" CACHE STRING "debug C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-O3 ${COMMON_C_FLAGS} -DNDEBUG ${EXTRA_CFLAGS}" CACHE STRING "release C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${COMMON_CXX_FLAGS} -DNDEBUG ${EXTRA_CXXFLAGS}" CACHE STRING "release C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -ggdb ${COMMON_C_FLAGS} -DNDEBUG -fno-omit-frame-pointer ${EXTRA_CFLAGS}" CACHE STRING "release w symbols C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -ggdb ${COMMON_CXX_FLAGS} -DNDEBUG -fno-omit-frame-pointer ${EXTRA_CXXFLAGS}" CACHE STRING "release w symbols C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_MINSIZEREL "-Os ${COMMON_C_FLAGS} -DNDEBUG ${EXTRA_CFLAGS}" CACHE STRING "minrelease C flags" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os ${COMMON_CXX_FLAGS} -DNDEBUG ${EXTRA_CXXFLAGS}" CACHE STRING "minrelease C++ flags" FORCE)

# This build disables all memory allocators and instead uses malloc/free for every allocation.  This can catch memory leaks obscured by the use of memory allocators.
set(CMAKE_C_FLAGS_DEBUGNOPOOL "-O0 -ggdb ${COMMON_C_FLAGS} -DESB_NO_ALLOC -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak ${EXTRA_CFLAGS}" CACHE STRING "debug no pool C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUGNOPOOL "-O0 -ggdb ${COMMON_CXX_FLAGS} -DESB_NO_ALLOC -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak ${EXTRA_CXXFLAGS}" CACHE STRING "debug no pool C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUGNOPOOL "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_RELEASENOPOOL "-O3 ${COMMON_C_FLAGS} -DESB_NO_ALLOC -DNDEBUG ${EXTRA_CFLAGS}" CACHE STRING "release no pool C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASENOPOOL "-O3 ${COMMON_CXX_FLAGS} -DESB_NO_ALLOC -DNDEBUG ${EXTRA_CXXFLAGS}" CACHE STRING "release no pool C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASENOPOOL "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_COVERAGE "-O0 -ggdb ${COMMON_C_FLAGS} -fno-omit-frame-pointer -fsanitize=leak -fprofile-instr-generate -fcoverage-mapping ${EXTRA_CFLAGS}" CACHE STRING "coverage C flags" FORCE)
set(CMAKE_CXX_FLAGS_COVERAGE "-O0 -ggdb ${COMMON_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=leak -fprofile-instr-generate -fcoverage-mapping ${EXTRA_CXXFLAGS}" CACHE STRING "coverage C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "-Wl,-export-dynamic -fprofile-instr-generate" CACHE STRING "coverage linker flags" FORCE)
