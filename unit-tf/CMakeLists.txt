cmake_minimum_required(VERSION 3.5)
project(unit-tf VERSION ${VERSION} LANGUAGES CXX)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

set(SOURCE_FILES
        source/ESTFDate.cpp
        source/ESTFConcurrencyDecorator.cpp
        source/ESTFConcurrencyComposite.cpp
        source/ESTFComponentThread.cpp
        source/ESTFComposite.cpp
        source/ESTFRandomizedDecorator.cpp
        source/ESTFRepetitionDecorator.cpp
        source/ESTFResultCollector.cpp
        source/ESTFThread.cpp
        )

add_library(unit-tf STATIC ${SOURCE_FILES})

target_include_directories(unit-tf PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        )

target_include_directories(unit-tf PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

target_compile_options(unit-tf PUBLIC ${CPP_FLAGS})
