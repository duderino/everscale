cmake_minimum_required(VERSION 3.11)
set(VERSION 0.0.1)
project(everscale VERSION ${VERSION} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(profiles.cmake) # To select profile: cmake -DCMAKE_BUILD_TYPE=DEBUG .
include(config.cmake)
include(dependencies.cmake)
include(macros.cmake)
include(CTest)

#
# subdirs
#

add_subdirectory(unit-tf)
add_subdirectory(base)
add_subdirectory(data-plane)
#add_subdirectory(control-plane) - disabling until actually needed to reduce build time and circleci bills
add_subdirectory(tests)

#
# format
#

if (CLANG_FORMAT)
    add_custom_target(format
            COMMAND ${CLANG_FORMAT} -i --style=\"{BasedOnStyle: Google, ColumnLimit: 120}\" ${FORMAT_FILES}
            )
endif ()

string(REPLACE ";" "," ALL_TESTS "${ALL_TESTS}")
string(REPLACE ";" " " PROFRAW_FILES "${PROFRAW_FILES}")

configure_file(CTestCustom.cmake.in CTestCustom.cmake)
