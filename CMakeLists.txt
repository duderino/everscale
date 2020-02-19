cmake_minimum_required(VERSION 3.5)
project(everscale VERSION ${VERSION} LANGUAGES CXX C)
include(config.cmake)

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CPP_FLAGS
	-ansi 
	-Wall 
	-Woverloaded-virtual 
	-Werror 
	-fno-exceptions 
	-fno-rtti 
	-DES_LINUX 
	-D_REENTRANT
)

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



