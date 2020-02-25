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

macro(add_unit_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_compile_options(${TEST_NAME} PUBLIC ${CPP_FLAGS} -DESTF_USE_RESULT_COLLECTOR)
  target_link_libraries(${TEST_NAME} -pthread unit-tf http1 http-common base)
  target_include_directories(${TEST_NAME} PRIVATE 
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/tests"
    "${PROJECT_SOURCE_DIR}/../http-common/include"
    "${PROJECT_SOURCE_DIR}/../../base/include"
    "${PROJECT_SOURCE_DIR}/../../unit-tf/include"
    "${PROJECT_SOURCE_DIR}/include"
  )
  # By default ctest eats stderr/out which makes debugging test failures inconvenient.
  # To see stderr/stdout on test failure, do a:
  #   CTEST_OUTPUT_ON_FAILURE=1 make test
  # Or add "export CTEST_OUTPUT_ON_FAILURE=1" to your .bashrc
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests")
  set_tests_properties(${TEST_NAME} PROPERTIES
    TIMEOUT 10
  )
endmacro()

add_unit_test(http1-parser-test tests/ESHttpParserTest.cpp)

