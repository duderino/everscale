cmake_minimum_required(VERSION 3.5)
project(base VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCE_FILES
    src/ESBAllocatorCleanupHandler.cpp
    src/ESBBuddyAllocator.cpp
    src/ESBBuffer.cpp
    src/ESBBufferPool.cpp
    src/ESBCommand.cpp
    src/ESBCommandThread.cpp
    src/ESBComparator.cpp
    src/ESBConsoleLogger.cpp
    src/ESBConnectedTCPSocket.cpp
    src/ESBCountingSemaphore.cpp
    src/ESBDate.cpp
    src/ESBEmbeddedListElement.cpp
    src/ESBEmbeddedList.cpp
    src/ESBError.cpp
    src/ESBFixedAllocator.cpp
    src/ESBFlag.cpp
    src/ESBList.cpp
    src/ESBListeningTCPSocket.cpp
    src/ESBLockable.cpp
    src/ESBMap.cpp
    src/ESBMutex.cpp
    src/ESBMultiplexedSocket.cpp
    src/ESBNullLock.cpp
    src/ESBNullLogger.cpp
    src/ESBRand.cpp
    src/ESBReadWriteLock.cpp
    src/ESBReferenceCount.cpp
    src/ESBSharedAllocator.cpp
    src/ESBSharedCounter.cpp
    src/ESBSharedEmbeddedQueue.cpp
    src/ESBSharedQueue.cpp
    src/ESBSmartPointer.cpp
    src/ESBSmartPointerDebugger.cpp
    src/ESBSocketMultiplexer.cpp
    src/ESBSocketMultiplexerDispatcher.cpp
    src/ESBSocketMultiplexerFactory.cpp
    src/ESBSystemAllocator.cpp
    src/ESBThread.cpp
    src/ESBThreadPool.cpp
    src/ESBSocketAddress.cpp
    src/ESBTCPSocket.cpp
    src/ESBDiscardAllocator.cpp
    src/ESBEpollMultiplexer.cpp
    src/ESBEpollMultiplexerFactory.cpp
)

add_library(base STATIC ${SOURCE_FILES})

target_include_directories(base PRIVATE 
       	"${PROJECT_SOURCE_DIR}/src"
       	"${PROJECT_SOURCE_DIR}/../unit-tf/include"
)

target_include_directories(base PUBLIC
       	"${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(base PUBLIC 
	-ansi 
	-Wall 
	-Woverloaded-virtual 
	-Werror 
	-fno-exceptions 
	-fno-rtti 
	-DES_LINUX 
	-D_REENTRANT
)

macro(add_unit_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} base unit-tf)
    target_include_directories(${TESTNAME} PRIVATE 
       	"${PROJECT_SOURCE_DIR}/src"
       	"${PROJECT_SOURCE_DIR}/../unit-tf/include"
    )
endmacro()

add_unit_test(buddy-allocator-test tests/ESBBuddyAllocatorTest.cpp)


