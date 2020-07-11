cmake_minimum_required(VERSION 3.5)
project(http-test-common VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
	source/ESHttpTestParams.cpp
	source/ESEphemeralListener.cpp
        )

add_library(http-test-common STATIC ${SOURCE_FILES})

target_include_directories(http-test-common PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(http-test-common PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(http-test-common PUBLIC ${CPP_FLAGS})

