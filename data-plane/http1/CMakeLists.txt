cmake_minimum_required(VERSION 3.5)
project(http1 VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESHttpClientCommand.cpp
        source/ESHttpClientCommandSocket.cpp
        source/ESHttpClientCounters.cpp
        source/ESHttpClientHandler.cpp
        source/ESHttpClientHistoricalCounters.cpp
        source/ESHttpClientSimpleCounters.cpp
        source/ESHttpClientSocket.cpp
        source/ESHttpClientSocketFactory.cpp
        source/ESHttpClientTransaction.cpp
        source/ESHttpClientTransactionFactory.cpp
        source/ESHttpCommandSocket.cpp
        source/ESHttpConnectionPool.cpp
        source/ESHttpMessageFormatter.cpp
        source/ESHttpMessageParser.cpp
        source/ESHttpMultiplexer.cpp
        source/ESHttpMultiplexerExtended.cpp
        source/ESHttpProxyHandler.cpp
        source/ESHttpRequestFormatter.cpp
        source/ESHttpRequestParser.cpp
        source/ESHttpRequestUriFormatter.cpp
        source/ESHttpRequestUriParser.cpp
        source/ESHttpResponseFormatter.cpp
        source/ESHttpResponseParser.cpp
        source/ESHttpServerCommand.cpp
        source/ESHttpServerCommandSocket.cpp
        source/ESHttpServerCounters.cpp
        source/ESHttpServerHandler.cpp
        source/ESHttpServerSimpleCounters.cpp
        source/ESHttpServerSocket.cpp
        source/ESHttpServerSocketFactory.cpp
        source/ESHttpServerTransaction.cpp
        source/ESHttpServerTransactionFactory.cpp
        source/ESHttpSocket.cpp
		source/ESHttpListeningSocket.cpp
		)

add_library(http1 STATIC ${SOURCE_FILES})

target_include_directories(http1 PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(http1 PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(http1 PUBLIC ${CPP_FLAGS})

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/source"
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

add_gtest(http1-parser-formatter-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESHttpParserFormatterTest.cpp)


