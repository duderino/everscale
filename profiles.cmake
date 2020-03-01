cmake_minimum_required(VERSION 3.11)

set(CMAKE_C_FLAGS_DEFAULT "-O2 -ggdb -Wall -Werror -D_REENTRANT")
set(CMAKE_CXX_FLAGS_DEFAULT "-O2 -ggdb -Wall -Werror -D_REENTRANT -fno-exceptions -fno-rtti")
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_DEFAULT CMAKE_C_FLAGS_DEFAULT CMAKE_EXE_LINKER_FLAGS_DEFAULT CMAKE_SHARED_LINKER_FLAGS_DEFAULT)

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb -Wall -Werror -D_REENTRANT")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-exceptions -fno-rtti")

set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Werror -D_REENTRANT -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Werror -D_REENTRANT -fno-exceptions -fno-rtti -DNDEBUG")

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb -Wall -Werror -D_REENTRANT -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb -Wall -Werror -D_REENTRANT -fno-exceptions -fno-rtti -DNDEBUG")

set(CMAKE_C_FLAGS_MINSIZEREL "-Os -Wall -Werror -D_REENTRANT -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -Wall -Werror -D_REENTRANT -fno-exceptions -fno-rtti -DNDEBUG")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Default" CACHE STRING "Choose the type of build, options are: Default Debug Release RelWithDebInfo MinSizeRel.")
endif()

