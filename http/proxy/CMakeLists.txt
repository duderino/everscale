cmake_minimum_required(VERSION 3.5)
project(proxy VERSION ${VERSION} LANGUAGES CXX)

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
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
        unit-tf
        loadgen
        origin
        multiplexers
        http1
        http-common
        base
        )

add_unit_test(http-client-server-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 60 tests/ESHttpClientServerTest.cpp ${CLIENT_FILES} ${SERVER_FILES})
