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
        src/ESBProcessLimits.cpp
        )

add_library(base STATIC ${SOURCE_FILES})

target_include_directories(base PRIVATE
        "${PROJECT_SOURCE_DIR}/src"
        )

target_include_directories(base PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

#target_compile_options(base PUBLIC ${CPP_FLAGS})

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../unit-tf/include"
        )

set(TEST_LIBS -pthread unit-tf base)

add_unit_test(buddy-allocator-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 1 tests/ESBBuddyAllocatorTest.cpp)
add_unit_test(buffer-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 5 tests/ESBBufferTest.cpp)
add_unit_test(discard-allocator-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 5 tests/ESBDiscardAllocatorTest.cpp)
add_unit_test(fixed-allocator-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 1 tests/ESBFixedAllocatorTest.cpp)
add_unit_test(list-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 5 tests/ESBListTest.cpp)
add_unit_test(lockable-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 1 tests/ESBLockableTest.cpp)
add_unit_test(map-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 10 tests/ESBMapTest.cpp)
add_unit_test(shared-counter-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 5 tests/ESBSharedCounterTest.cpp)
add_unit_test(smart-pointer-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 1 tests/ESBSmartPointerTest.cpp)
add_unit_test(shared-queue-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 5 tests/ESBSharedQueueTest.cpp tests/ESBSharedQueueConsumer.cpp tests/ESBSharedQueueProducer.cpp)

add_gtest(date-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBDateTest.cpp)
add_gtest(simple-performance-counter-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBSimplePerformanceCounterTest.cpp)
add_gtest(process-limits-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBProcessLimitsTest.cpp)
