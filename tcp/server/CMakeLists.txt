cmake_minimum_required(VERSION 3.5)
project(tcp_server VERSION ${VERSION} LANGUAGES CXX)

add_library(tcp_server STATIC 
  ESTListeningSocket.cpp
  ESTServerSocket.cpp
)
target_include_directories(tcp_server PRIVATE 
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/../../base/include"
)
target_compile_options(tcp_server PUBLIC ${CPP_FLAGS})

add_executable(tcp_server.exe ESTServerMain.cpp)
target_include_directories(tcp_server.exe PRIVATE 
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/../../base/include"
)
target_link_libraries(tcp_server.exe -pthread tcp_server base)
target_compile_options(tcp_server.exe PUBLIC ${CPP_FLAGS})
