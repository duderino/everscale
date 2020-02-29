cmake_minimum_required(VERSION 3.5)
project(http_stack VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        src/ESHttpStack.cpp
        )

add_library(http_stack STATIC ${SOURCE_FILES})

target_include_directories(http_stack PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        )

target_include_directories(http_stack PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(http_stack PUBLIC ${CPP_FLAGS})

set(CLIENT_FILES
        tests/ESHttpEchoClientContext.cpp
        tests/ESHttpEchoClientHandler.cpp
        tests/ESHttpEchoClientRequestBuilder.cpp
        )

add_executable(http_client.exe ${CLIENT_FILES} tests/ESHttpEchoClientMain.cpp)
target_compile_options(http_client.exe PUBLIC ${CPP_FLAGS})
target_link_libraries(http_client.exe -pthread unit-tf http_stack http1 http-common base)
target_include_directories(http_client.exe PRIVATE
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../stack/include"
        )

set(SERVER_FILES
        tests/ESHttpEchoServerContext.cpp
        tests/ESHttpEchoServerHandler.cpp
        )

add_executable(http_server.exe ${SERVER_FILES} tests/ESHttpEchoServerMain.cpp)
target_compile_options(http_server.exe PUBLIC ${CPP_FLAGS})
target_link_libraries(http_server.exe -pthread unit-tf http_stack http1 http-common base)
target_include_directories(http_server.exe PRIVATE
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../stack/include"
        )

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../stack/include"
        )

set(TEST_LIBS
        -pthread
        unit-tf
        http_stack
        http1
        http-common
        base
        )

add_unit_test(http-client-server-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 60 tests/ESHttpClientServerTest.cpp ${CLIENT_FILES} ${SERVER_FILES})
