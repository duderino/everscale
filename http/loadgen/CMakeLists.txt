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
        )

target_compile_options(loadgen PUBLIC ${CPP_FLAGS})

add_executable(http-loadgen source/ESHttpLoadgen.cpp)
target_compile_options(http-loadgen PUBLIC ${CPP_FLAGS})
target_link_libraries(http-loadgen -pthread unit-tf loadgen multiplexers http1 http-common http-test-common base bssl_ssl bssl_crypto)
target_include_directories(http-loadgen PRIVATE
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http-test-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        "${PROJECT_SOURCE_DIR}/../multiplexers/include"
        "${PROJECT_SOURCE_DIR}/include"
        )
