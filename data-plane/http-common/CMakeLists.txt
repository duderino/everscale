cmake_minimum_required(VERSION 3.5)
project(http-common VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESHttpClientStream.cpp
        source/ESHttpHeader.cpp
        source/ESHttpMessage.cpp
        source/ESHttpRequest.cpp
        source/ESHttpRequestUri.cpp
        source/ESHttpResponse.cpp
        source/ESHttpTransaction.cpp
        source/ESHttpUtil.cpp
        source/ESHttpConfig.cpp
        source/ESHttpServerStream.cpp
        source/ESHttpStream.cpp
        )

add_library(http-common STATIC ${SOURCE_FILES})

target_include_directories(http-common PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(http-common PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/../http-plugin/include"
        )

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../data-plane/http-plugin/include"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../../unit-tf/include"
        )

set(TEST_LIBS -pthread unit-tf http-common config base)

add_gtest(http-common-alignment-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/AlignmentTest.cpp)

# For global code coverage report

set(ALL_TESTS ${ALL_TESTS} ${TESTS} PARENT_SCOPE)
set(PROFRAW_FILES "${PROFRAW_FILES}" "${PROJECT_SOURCE_DIR}/tests/*.profraw" PARENT_SCOPE)