cmake_minimum_required(VERSION 3.5)
project(base VERSION ${VERSION} LANGUAGES CXX)

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
    src/ESBDnsClient.cpp
    src/ESBSystemDnsClient.cpp
    src/ESBAveragingCounter.cpp
    src/ESBPerformanceCounter.cpp
    src/ESBHistoricalPerformanceCounter.cpp
    src/ESBSimplePerformanceCounter.cpp
)

add_library(base STATIC ${SOURCE_FILES})

target_include_directories(base PRIVATE 
  "${PROJECT_SOURCE_DIR}/src"
)

target_include_directories(base PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
)

target_compile_options(base PUBLIC ${CPP_FLAGS})

# Unit tests

macro(add_unit_test TEST_NAME)
  add_executable(${TEST_NAME} ${ARGN})
  target_compile_options(${TEST_NAME} PUBLIC ${CPP_FLAGS} -DESTF_USE_RESULT_COLLECTOR -DUSE_SMART_POINTER_DEBUGGER)
  target_link_libraries(${TEST_NAME} -pthread unit-tf base)
  target_include_directories(${TEST_NAME} PRIVATE 
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/tests"
    "${PROJECT_SOURCE_DIR}/../unit-tf/include"
  )
  # By default ctest eats stderr/out which makes debugging test failures inconvenient.
  # To see stderr/stdout on test failure, do a:
  #   CTEST_OUTPUT_ON_FAILURE=1 make test
  # Or add "export CTEST_OUTPUT_ON_FAILURE=1" to your .bashrc
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
  set_tests_properties(${TEST_NAME} PROPERTIES
    TIMEOUT 10
  )
  set_property(TEST ${TEST_NAME} APPEND PROPERTY CTEST_OUTPUT_ON_FAILURE=1)
endmacro()

add_unit_test(buddy-allocator-test tests/ESBBuddyAllocatorTest.cpp)
add_unit_test(buffer-test tests/ESBBufferTest.cpp)
add_unit_test(discard-allocator-test tests/ESBDiscardAllocatorTest.cpp)
add_unit_test(fixed-allocator-test tests/ESBFixedAllocatorTest.cpp)
add_unit_test(list-test tests/ESBListTest.cpp)
add_unit_test(lockable-test tests/ESBLockableTest.cpp)
add_unit_test(map-test tests/ESBMapTest.cpp)
add_unit_test(shared-counter-test tests/ESBSharedCounterTest.cpp)
add_unit_test(smart-pointer-test tests/ESBSmartPointerTest.cpp)
add_unit_test(shared-queue-test tests/ESBSharedQueueTest.cpp tests/ESBSharedQueueConsumer.cpp tests/ESBSharedQueueProducer.cpp)

