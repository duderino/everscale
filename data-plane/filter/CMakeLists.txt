cmake_minimum_required(VERSION 3.5)
project(http-filter VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESHttpFilter.cpp
        source/ESHttpConnectionFilter.cpp
        )

add_library(http-filter STATIC ${SOURCE_FILES})

target_include_directories(http-filter PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(http-filter PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../../unit-tf/include"
        )

set(TEST_LIBS -pthread unit-tf http-filter base)

#add_gtest(http-common-alignment-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/AlignmentTest.cpp)

# For global code coverage report

set(ALL_TESTS ${ALL_TESTS} ${TESTS} PARENT_SCOPE)
set(PROFRAW_FILES "${PROFRAW_FILES}" "${PROJECT_SOURCE_DIR}/tests/*.profraw" PARENT_SCOPE)
