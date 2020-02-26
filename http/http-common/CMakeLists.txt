cmake_minimum_required(VERSION 3.5)
project(http-common VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
        src/ESHttpHeader.cpp
        src/ESHttpMessage.cpp
        src/ESHttpRequest.cpp
        src/ESHttpRequestUri.cpp
        src/ESHttpResponse.cpp
        src/ESHttpTransaction.cpp
        src/ESHttpUtil.cpp
        )

add_library(http-common STATIC ${SOURCE_FILES})

target_include_directories(http-common PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(http-common PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(http-common PUBLIC ${CPP_FLAGS})
