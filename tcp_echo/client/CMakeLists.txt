cmake_minimum_required(VERSION 3.5)
project(tcp_client VERSION ${VERSION} LANGUAGES CXX)

add_library(tcp_client STATIC 
  ESTClientSocket.cpp
  ESTClientSocketFactory.cpp
  ESTPerformanceCounter.cpp
  ESTClientMain.cpp
)
target_include_directories(tcp_client PRIVATE 
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/../../base/include"
)
target_compile_options(tcp_client PUBLIC ${CPP_FLAGS})

add_executable(tcp_client.exe ESTClientMain.cpp)
target_include_directories(tcp_client.exe PRIVATE 
  "${PROJECT_SOURCE_DIR}/"
  "${PROJECT_SOURCE_DIR}/../../base/include"
)
target_link_libraries(tcp_client.exe -pthread tcp_client base)
target_compile_options(tcp_client.exe PUBLIC ${CPP_FLAGS})
