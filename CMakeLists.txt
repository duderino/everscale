cmake_minimum_required(VERSION 3.11)
set(VERSION 0.0.1)
project(everscale VERSION ${VERSION} LANGUAGES CXX C)
enable_testing()
include(config.cmake)
include(FetchContent)

SET(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_STANDARD 11)
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

#
# clang-format
#

find_program(CLANG_FORMAT "clang-format")

if (NOT CLANG_FORMAT)
    message(STATUS "clang-format not found.")
else ()
    message(STATUS "clang-format found: ${CLANG_FORMAT}")
endif ()

if (CLANG_FORMAT)
    FILE(GLOB_RECURSE FORMAT_FILES . *.cpp *.h)
    add_custom_target(format
            COMMAND ${CLANG_FORMAT} -i --style=Google ${FORMAT_FILES}
            )
endif ()

#
# googletest
#

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.10.x
)

FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif ()

include(GoogleTest)

mark_as_advanced(
        BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
        gmock_build_tests gtest_build_samples gtest_build_tests
        gtest_enable_pthreads
)

macro(add_gtest NAME INCS LIBS CWD)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} gtest gmock gtest_main ${LIBS})
    gtest_discover_tests(${NAME}
            WORKING_DIRECTORY ${CWD}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CWD}"
            )
    set_target_properties(${NAME} PROPERTIES FOLDER tests)
endmacro()

#
# unit-tf unit tests
#

macro(add_unit_test NAME INCS LIBS CWD TIMEOUT)
    add_executable(${NAME} ${ARGN})
    target_compile_options(${NAME} PUBLIC ${CPP_FLAGS} -DESTF_USE_RESULT_COLLECTOR -DUSE_SMART_POINTER_DEBUGGER)
    target_link_libraries(${NAME} ${LIBS})
    target_include_directories(${NAME} PRIVATE ${INCS})
    # By default ctest eats stderr/out which makes debugging test failures inconvenient.
    # To see stderr/stdout on test failure, do a:
    #   CTEST_OUTPUT_ON_FAILURE=1 make test
    # Or add "export CTEST_OUTPUT_ON_FAILURE=1" to your .bashrc
    add_test(NAME ${NAME} COMMAND ${NAME} WORKING_DIRECTORY ${CWD})
    set_tests_properties(${NAME} PROPERTIES
            TIMEOUT ${TIMEOUT}
            )
endmacro()

#
# subdirs
#

add_subdirectory(unit-tf)
add_subdirectory(base)
add_subdirectory(tcp)
add_subdirectory(http)
