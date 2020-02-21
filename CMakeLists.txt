cmake_minimum_required(VERSION 3.5)
project(everscale VERSION ${VERSION} LANGUAGES CXX C)
#enable_testing()
include( CTest )
include(config.cmake)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# TODO add -Wextra and make this portable
set(CPP_FLAGS
	-ansi 
	-Wall 
	-Woverloaded-virtual 
	-Werror 
	-fno-exceptions 
	-fno-rtti 
	-D_REENTRANT
)

# TODO make this portable
set(LDFLAGS -pthread)

find_program(CLANG_FORMAT "clang-format")

if(NOT CLANG_FORMAT)
  message(STATUS "clang-format not found.")
else()
  message(STATUS "clang-format found: ${CLANG_FORMAT}")
endif()

if(CLANG_FORMAT)
  FILE(GLOB_RECURSE FORMAT_FILES . *.cpp *.h)
  add_custom_target(format
	  COMMAND ${CLANG_FORMAT} -i --style=Google ${FORMAT_FILES}
  )
endif()

add_subdirectory(unit-tf)
add_subdirectory(base)
add_subdirectory(tcp_echo)



