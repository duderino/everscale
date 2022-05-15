cmake_minimum_required(VERSION 3.5)
project(multiplexers VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        source/ESHttpClient.cpp
        source/ESHttpProxy.cpp
        source/ESHttpProxyMultiplexer.cpp
        source/ESHttpServer.cpp
        )

add_library(multiplexers STATIC ${SOURCE_FILES})

target_include_directories(multiplexers PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        )

target_include_directories(multiplexers PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/../http-plugin/include"
        )

