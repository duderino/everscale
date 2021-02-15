cmake_minimum_required(VERSION 3.5)
project(control VERSION ${VERSION} LANGUAGES CXX)

include(dependencies.cmake)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

set(SOURCE_FILES
        source/Nothing.cpp
        )

add_library(control STATIC ${SOURCE_FILES})

target_include_directories(control PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${CMAKE_SOURCE_DIR}/base/include"
        "${CMAKE_SOURCE_DIR}/http/http-common/include"
        "${CMAKE_SOURCE_DIR}/http/http1/include"
        "${CMAKE_SOURCE_DIR}/http/multiplexers/include"
        "${CMAKE_SOURCE_DIR}/http/proxy/include"
        )

target_include_directories(control PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/tests"
        "${CMAKE_SOURCE_DIR}/http/http-test-common/include"
        "${CMAKE_SOURCE_DIR}/http/proxy/tests"
        "${CMAKE_SOURCE_DIR}/http/loadgen/include"
        "${CMAKE_SOURCE_DIR}/http/origin/include"
        "${CMAKE_SOURCE_DIR}/unit-tf/include"
        )

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
