cmake_minimum_required(VERSION 3.5)
project(http1 VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        src/ESHttpClientCounters.cpp
        src/ESHttpClientHandler.cpp
        src/ESHttpClientHistoricalCounters.cpp
        src/ESHttpClientSimpleCounters.cpp
        src/ESHttpClientSocket.cpp
        src/ESHttpClientSocketFactory.cpp
        src/ESHttpClientTransaction.cpp
        src/ESHttpClientTransactionFactory.cpp
        src/ESHttpConnectionPool.cpp
        src/ESHttpListeningSocket.cpp
        src/ESHttpMessageFormatter.cpp
        src/ESHttpMessageParser.cpp
        src/ESHttpRequestFormatter.cpp
        src/ESHttpRequestParser.cpp
        src/ESHttpRequestUriFormatter.cpp
        src/ESHttpRequestUriParser.cpp
        src/ESHttpResponseFormatter.cpp
        src/ESHttpResponseParser.cpp
        src/ESHttpServerCounters.cpp
        src/ESHttpServerHandler.cpp
        src/ESHttpServerSimpleCounters.cpp
        src/ESHttpServerSocket.cpp
        src/ESHttpServerSocketFactory.cpp
        src/ESHttpServerTransaction.cpp
        )

add_library(http1 STATIC ${SOURCE_FILES})

target_include_directories(http1 PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(http1 PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(http1 PUBLIC ${CPP_FLAGS})

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../../unit-tf/include"
        "${PROJECT_SOURCE_DIR}/include"
        )

SET(TEST_LIBS
        -pthread
        unit-tf
        http1
        http-common
        base
        )

add_unit_test(http1-parser-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 5 tests/ESHttpParserTest.cpp)
#add_gtest(http1-alignment-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/AlignmentTest.cpp)


