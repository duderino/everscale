cmake_minimum_required(VERSION 3.11)

set(CMAKE_BUILD_TYPE RELEASE)

set(VALID_BUILD_TYPES DEFAULT ASAN TSAN DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)

set(CMAKE_C_FLAGS_DEFAULT "-O2 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer" CACHE STRING "default C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEFAULT "-O2 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti" CACHE STRING "default C++ flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_DEFAULT CMAKE_C_FLAGS_DEFAULT CMAKE_EXE_LINKER_FLAGS_DEFAULT CMAKE_SHARED_LINKER_FLAGS_DEFAULT)

set(CMAKE_C_FLAGS_ASAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fsanitize=address -fsanitize=leak" CACHE STRING "asan C flags" FORCE)
set(CMAKE_CXX_FLAGS_ASAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti -fsanitize=address -fsanitize=leak" CACHE STRING "asan C++ flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_ASAN CMAKE_C_FLAGS_ASAN CMAKE_EXE_LINKER_FLAGS_ASAN CMAKE_SHARED_LINKER_FLAGS_ASAN)

set(CMAKE_C_FLAGS_TSAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fsanitize=thread" CACHE STRING "tsan C flags" FORCE)
set(CMAKE_CXX_FLAGS_TSAN "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti -fsanitize=thread" CACHE STRING "tsan C++ flags" FORCE)
MARK_AS_ADVANCED(CMAKE_CXX_FLAGS_TSAN CMAKE_C_FLAGS_TSAN CMAKE_EXE_LINKER_FLAGS_TSAN CMAKE_SHARED_LINKER_FLAGS_TSAN)

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fsanitize=leak" CACHE STRING "debug C flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -Wall -Werror -D_REENTRANT -fno-omit-frame-pointer -fno-exceptions -fno-rtti -fsanitize=leak" CACHE STRING "debug C++ flags" FORCE)

set(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -Werror -D_REENTRANT -DNDEBUG" CACHE STRING "release C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -Werror -D_REENTRANT -DNDEBUG -fno-exceptions -fno-rtti" CACHE STRING "release C++ flags" FORCE)

set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -ggdb -Wall -Werror -D_REENTRANT -DNDEBUG -fno-omit-frame-pointer" CACHE STRING "release w symbols C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -ggdb -Wall -Werror -D_REENTRANT -DNDEBUG -fno-omit-frame-pointer -fno-exceptions -fno-rtti" CACHE STRING "release w symbols C++ flags" FORCE)

set(CMAKE_C_FLAGS_MINSIZEREL "-Os -Wall -Werror -D_REENTRANT -DNDEBUG" CACHE STRING "minrelease C flags" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -Wall -Werror -D_REENTRANT -DNDEBUG -fno-exceptions -fno-rtti" CACHE STRING "minrelease C++ flags" FORCE)

if(NOT CMAKE_BUILD_TYPE IN_LIST VALID_BUILD_TYPES)
    message(FATAL_ERROR "Pick a build type: cmake -DCMAKE_BUILD_TYPE=(DEFAULT|ASAN|TSAN|DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)")
else()
    message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
endif()


