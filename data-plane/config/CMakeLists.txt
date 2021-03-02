cmake_minimum_required(VERSION 3.5)
project(config VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESWildcardIndex.cpp
        source/ESConfig.cpp
        source/ESServiceEntry.cpp
        source/ESDestinationRule.cpp
        source/ESVirtualService.cpp
        source/ESGateway.cpp
        source/ESSidecar.cpp
        )

add_library(config STATIC ${SOURCE_FILES})

target_include_directories(config PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(config PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../../unit-tf/include"
        "${CMAKE_SOURCE_DIR}/data-plane/config/include"
        )

SET(TEST_LIBS
        -pthread
        config
        unit-tf
        base
        )

add_gtest(wildcard-index-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESWildcardIndexTest.cpp)

