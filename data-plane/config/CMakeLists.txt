cmake_minimum_required(VERSION 3.5)
project(config VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
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

#target_compile_options(config PUBLIC ${CPP_FLAGS})
