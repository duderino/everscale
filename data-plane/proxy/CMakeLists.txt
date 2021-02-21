cmake_minimum_required(VERSION 3.5)
project(proxy VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESHttpRouter.cpp
        source/ESSynchronousDnsHttpRouter.cpp
        source/ESHttpRoutingProxyContext.cpp
        source/ESHttpRoutingProxyHandler.cpp
        )

add_library(proxy STATIC ${SOURCE_FILES})

target_include_directories(proxy PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        "${PROJECT_SOURCE_DIR}/../loadgen/include"
        "${PROJECT_SOURCE_DIR}/../origin/include"
        )

target_include_directories(proxy PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(proxy PUBLIC ${CPP_FLAGS})

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-test-common/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        "${PROJECT_SOURCE_DIR}/../loadgen/include"
        "${PROJECT_SOURCE_DIR}/../origin/include"
        "${PROJECT_SOURCE_DIR}/include"
        )

#        -ltcmalloc
set(TEST_LIBS
        -pthread
        -ldl
        unit-tf
        proxy
        loadgen
        origin
        multiplexers
        http1
        http-common
        http-test-common
        base
        bssl_ssl
        bssl_crypto
        )

set(TEST_FILES
        tests/ESHttpIntegrationTest.cpp
        tests/ESHttpFixedRouter.cpp
        )

add_unit_test(http-proxy-test-main "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESHttpProxyTestMain.cpp ${TEST_FILES})
add_gtest(http-proxy-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 550 tests/ESHttpProxyTest.cpp ${TEST_FILES})
add_gtest(http-proxy-negative-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 550 tests/ESHttpProxyNegativeTest.cpp ${TEST_FILES})
