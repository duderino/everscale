cmake_minimum_required(VERSION 3.5)
project(http-common VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        src/ESHttpHeader.cpp
        src/ESHttpMessage.cpp
        src/ESHttpRequest.cpp
        src/ESHttpRequestUri.cpp
        src/ESHttpResponse.cpp
        src/ESHttpTransaction.cpp
        src/ESHttpUtil.cpp
        src/ESHttpConfig.cpp
        )

add_library(http-common STATIC ${SOURCE_FILES})

target_include_directories(http-common PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(http-common PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(http-common PUBLIC ${CPP_FLAGS})

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../../unit-tf/include"
        )

set(TEST_LIBS -pthread unit-tf base http-common)

add_gtest(http-common-alignment-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/AlignmentTest.cpp)

