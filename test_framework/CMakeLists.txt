cmake_minimum_required(VERSION 3.5)
project(estf VERSION 0.1.0 LANGUAGES CXX)
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

add_library(estf STATIC ${SOURCE_FILES})

target_include_directories(estf PRIVATE 
       	"${PROJECT_SOURCE_DIR}/include"
)

target_include_directories(estf PUBLIC
       	"${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(estf PUBLIC 
	-ansi 
	-Wall 
	-Woverloaded-virtual 
	-Werror 
	-fno-exceptions 
	-fno-rtti 
	-DES_LINUX 
	-D_REENTRANT
)
