cmake_minimum_required(VERSION 3.5)
project(loadgen VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
source/ESHttpLoadgenContext.cpp
source/ESHttpLoadgenHandler.cpp
source/ESHttpLoadgenSeedCommand.cpp
        )

add_library(loadgen STATIC ${SOURCE_FILES})

target_include_directories(loadgen PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http-test-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        )

target_include_directories(loadgen PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/../http-plugin/include"
        )

add_executable(http-loadgen source/ESHttpLoadgen.cpp)
target_link_libraries(http-loadgen -pthread -ldl unit-tf loadgen multiplexers http1 http-common http-test-common config base bssl_ssl bssl_crypto)
target_include_directories(http-loadgen PRIVATE
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http-plugin/include"
        "${PROJECT_SOURCE_DIR}/../http-test-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        "${PROJECT_SOURCE_DIR}/include"
        )
