cmake_minimum_required(VERSION 3.11)

unset(DEFAULT_BUILD_TYPE)
set(DEFAULT_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE})
set(CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE} CACHE STRING "build type" FORCE)

if (DEFINED ENV{BUILD_TYPE})
  set(CMAKE_BUILD_TYPE $ENV{BUILD_TYPE})
  set(CMAKE_BUILD_TYPE $ENV{BUILD_TYPE} CACHE STRING "build type" FORCE)
endif()

set(VALID_BUILD_TYPES DEFAULT ASAN TSAN DEBUG RELEASE RELWITHDEBINFO MINSIZEREL DEBUGNOPOOL RELEASENOPOOL)
if(NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "ENV{BUILD_TYPE} '${CMAKE_BUILD_TYPE}' not in (DEFAULT|ASAN|TSAN|DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL|DEBUGNOPOOL|RELEASENOPOOL)")
else()
    message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_C_FLAGS_DEFAULT "-O2 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer" CACHE STRING "default C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEFAULT "-O2 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti" CACHE STRING "default C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEFAULT "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_DEFAULT CMAKE_C_FLAGS_DEFAULT CMAKE_EXE_LINKER_FLAGS_DEFAULT CMAKE_SHARED_LINKER_FLAGS_DEFAULT)

set(CMAKE_C_FLAGS_ASAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak" CACHE STRING "asan C flags" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti -fsanitize=address -fsanitize=leak" CACHE STRING "asan C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_ASAN "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_ASAN CMAKE_C_FLAGS_ASAN CMAKE_EXE_LINKER_FLAGS_ASAN CMAKE_SHARED_LINKER_FLAGS_ASAN)

set(CMAKE_C_FLAGS_TSAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fsanitize=thread" CACHE STRING "tsan C flags" FORCE)
set(CMAKE_CXX_FLAGS_TSAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti -fsanitize=thread" CACHE STRING "tsan C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_TSAN "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_TSAN CMAKE_C_FLAGS_TSAN CMAKE_EXE_LINKER_FLAGS_TSAN CMAKE_SHARED_LINKER_FLAGS_TSAN)

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fsanitize=leak" CACHE STRING "debug C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti -fsanitize=leak" CACHE STRING "debug C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Werror -D_REENTRANT -DNDEBUG" CACHE STRING "release C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Werror -D_REENTRANT -DNDEBUG -fno-exceptions -fno-rtti" CACHE STRING "release C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb -Wall -Werror -D_REENTRANT -DNDEBUG -fno-omit-frame-pointer" CACHE STRING "release w symbols C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb -Wall -Werror -D_REENTRANT -DNDEBUG -fno-omit-frame-pointer -fno-exceptions -fno-rtti" CACHE STRING "release w symbols C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_MINSIZEREL "-Os -Wall -Werror -D_REENTRANT -DNDEBUG" CACHE STRING "minrelease C flags" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -Wall -Werror -D_REENTRANT -DNDEBUG -fno-exceptions -fno-rtti" CACHE STRING "minrelease C++ flags" FORCE)

# This build disables all memory allocators and instead uses malloc/free for every allocation.  This can catch memory leaks obscured by the use of memory allocators.
set(CMAKE_C_FLAGS_DEBUGNOPOOL "-O0 -ggdb -Wall -Werror -D_REENTRANT -DESB_NO_ALLOC -fno-omit-frame-pointer -fsanitize=leak" CACHE STRING "debug no pool C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUGNOPOOL "-O0 -ggdb -Wall -Werror -D_REENTRANT -DESB_NO_ALLOC -fno-omit-frame-pointer -fno-exceptions -fno-rtti -fsanitize=leak" CACHE STRING "debug no pool C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUGNOPOOL "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)

set(CMAKE_C_FLAGS_RELEASENOPOOL "-O2 -Wall -Werror -D_REENTRANT -DESB_NO_ALLOC -DNDEBUG" CACHE STRING "release no pool C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASENOPOOL "-O2 -Wall -Werror -D_REENTRANT -DESB_NO_ALLOC -DNDEBUG -fno-exceptions -fno-rtti" CACHE STRING "release no pool C++ flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASENOPOOL "-Wl,-export-dynamic" CACHE STRING "default linker flags" FORCE)
