cmake_minimum_required(VERSION 3.5)
project(proxy VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESHttpRouter.cpp
        source/ESSynchronousDnsHttpRouter.cpp
        source/ESHttpRoutingProxyContext.cpp
        source/ESHttpRoutingProxyHandler.cpp
        source/ESHttpFixedRouter.cpp
        )

set(INCS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http-test-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        "${PROJECT_SOURCE_DIR}/../loadgen/include"
        "${PROJECT_SOURCE_DIR}/../origin/include"
        )

set(LIBS
        -pthread
        -ldl
        proxy
        loadgen
        origin
        multiplexers
        http1
        http-common
        http-test-common
        config
        base
        bssl_ssl
        bssl_crypto
        )

add_library(proxy STATIC ${SOURCE_FILES})
target_include_directories(proxy PRIVATE ${INCS})
target_include_directories(proxy PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/../http-plugin/include"
        )

add_executable(http-proxy source/ESHttpProxyMain.cpp)
target_link_libraries(http-proxy ${LIBS})
target_include_directories(http-proxy PRIVATE ${INCS})

# Unit tests

set(TEST_INCS
        ${INCLUDES}
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/tests"
        )

set(TEST_LIBS
        ${LIBS}
        unit-tf
        )

set(TEST_FILES tests/ESHttpIntegrationTest.cpp)

# Timeout values should be greater than those in ESHttpIntegrationTest.h.  The tests handle
# their own timeout so cmake will dump their output when timeout is hit.  When cmake handles
# the timeout the test output is lost.
add_unit_test(http-proxy-test-main "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 900 tests/ESHttpProxyTestMain.cpp ${TEST_FILES})
add_gtest(http-proxy-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 900 tests/ESHttpProxyTest.cpp ${TEST_FILES})
add_gtest(http-proxy-negative-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 900 tests/ESHttpProxyNegativeTest.cpp ${TEST_FILES})

# For global code coverage report

set(ALL_TESTS ${ALL_TESTS} ${TESTS} PARENT_SCOPE)
set(PROFRAW_FILES "${PROFRAW_FILES}" "${PROJECT_SOURCE_DIR}/tests/*.profraw" PARENT_SCOPE)
