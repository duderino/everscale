cmake_minimum_required(VERSION 3.5)
project(http-plugin VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESHttpPlugin.cpp
        )

add_library(http-plugin STATIC ${SOURCE_FILES})

target_include_directories(http-plugin PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        )

target_include_directories(http-plugin PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        )

set(TEST_LIBS -pthread http-plugin http1 http-common config base)

add_gtest(http-plugin-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESPluginTest.cpp)

# For global code coverage report

set(ALL_TESTS ${ALL_TESTS} ${TESTS} PARENT_SCOPE)
set(PROFRAW_FILES "${PROFRAW_FILES}" "${PROJECT_SOURCE_DIR}/tests/*.profraw" PARENT_SCOPE)
