cmake_minimum_required(VERSION 3.5)
project(multiplexers VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
src/ESHttpClient.cpp
src/ESHttpClientMultiplexer.cpp
src/ESHttpMultiplexer.cpp
src/ESHttpProxy.cpp
src/ESHttpProxyMultiplexer.cpp
src/ESHttpServer.cpp
src/ESHttpServerMultiplexer.cpp
        )

add_library(multiplexers STATIC ${SOURCE_FILES})

target_include_directories(multiplexers PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../http-common/include"
        "${PROJECT_SOURCE_DIR}/../http1/include"
        )

target_include_directories(multiplexers PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(multiplexers PUBLIC ${CPP_FLAGS})

