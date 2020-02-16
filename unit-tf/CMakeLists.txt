cmake_minimum_required(VERSION 3.5)
project(unit-tf VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES
	src/ESTFDate.cpp
	src/ESTFConcurrencyDecorator.cpp
	src/ESTFConcurrencyComposite.cpp
	src/ESTFComponentThread.cpp
	src/ESTFComposite.cpp
	src/ESTFRandomizedDecorator.cpp
	src/ESTFRepetitionDecorator.cpp
	src/ESTFResultCollector.cpp
	src/ESTFThread.cpp
)

add_library(unit-tf STATIC ${SOURCE_FILES})

target_include_directories(unit-tf PRIVATE 
       	"${PROJECT_SOURCE_DIR}/src"
)

target_include_directories(unit-tf PUBLIC
       	"${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(unit-tf PUBLIC 
	-ansi 
	-Wall 
	-Woverloaded-virtual 
	-Werror 
	-fno-exceptions 
	-fno-rtti 
	-DES_LINUX 
	-D_REENTRANT
)
