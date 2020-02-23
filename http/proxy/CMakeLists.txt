cmake_minimum_required(VERSION 3.5)
project(http_stack VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
  src/ESHttpStack.cpp
)

add_library(http_stack STATIC ${SOURCE_FILES})

target_include_directories(http_stack PRIVATE 
  "${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/../../base/include"
  "${PROJECT_SOURCE_DIR}/../http-common/include"
  "${PROJECT_SOURCE_DIR}/../http1/include"
)

target_include_directories(http_stack PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(http_stack PUBLIC ${CPP_FLAGS})
