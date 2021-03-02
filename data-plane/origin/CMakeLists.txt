cmake_minimum_required(VERSION 3.5)
project(origin VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
source/ESHttpOriginContext.cpp
source/ESHttpOriginHandler.cpp
        )

add_library(origin STATIC ${SOURCE_FILES})

target_include_directories(origin PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http-test-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        )

target_include_directories(origin PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

add_executable(http-origin source/ESHttpOrigin.cpp)
target_link_libraries(http-origin -pthread -ldl unit-tf origin multiplexers http1 http-common http-test-common config base bssl_ssl bssl_crypto)
target_include_directories(http-origin PRIVATE
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http-test-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        "${PROJECT_SOURCE_DIR}/include"
        )
