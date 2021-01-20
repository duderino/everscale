cmake_minimum_required(VERSION 3.5)
project(base VERSION ${VERSION} LANGUAGES CXX)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

set(SOURCE_FILES
        source/ESBAllocator.cpp
        source/ESBBuddyAllocator.cpp
        source/ESBBuffer.cpp
        source/ESBBufferPool.cpp
        source/ESBClearSocket.cpp
        source/ESBCommand.cpp
        source/ESBCommandThread.cpp
        source/ESBComparator.cpp
        source/ESBSimpleFileLogger.cpp
        source/ESBConnectedSocket.cpp
        source/ESBTLSSocket.cpp
        source/ESBClientTLSSocket.cpp
        source/ESBServerTLSSocket.cpp
        source/ESBConnectionPool.cpp
        source/ESBCountingSemaphore.cpp
        source/ESBDate.cpp
        source/ESBEmbeddedListElement.cpp
        source/ESBEmbeddedList.cpp
        source/ESBError.cpp
        source/ESBFixedAllocator.cpp
        source/ESBHostAddress.cpp
        source/ESBList.cpp
        source/ESBListeningSocket.cpp
        source/ESBLockable.cpp
        source/ESBLogger.cpp
        source/ESBMap.cpp
        source/ESBMutex.cpp
        source/ESBMultiplexedSocket.cpp
        source/ESBNullLock.cpp
        source/ESBNullLogger.cpp
        source/ESBRand.cpp
        source/ESBReadWriteLock.cpp
        source/ESBReferenceCount.cpp
        source/ESBSharedAllocator.cpp
        source/ESBSharedInt.cpp
        source/ESBSharedEmbeddedQueue.cpp
        source/ESBSharedQueue.cpp
        source/ESBSignalHandler.cpp
        source/ESBSmartPointer.cpp
        source/ESBSmartPointerDebugger.cpp
        source/ESBSocketMultiplexer.cpp
        source/ESBString.cpp
        source/ESBSystemAllocator.cpp
        source/ESBThread.cpp
        source/ESBThreadPool.cpp
        source/ESBSocketAddress.cpp
        source/ESBSocket.cpp
        source/ESBDiscardAllocator.cpp
        source/ESBEpollMultiplexer.cpp
        source/ESBDnsClient.cpp
        source/ESBSystemDnsClient.cpp
        source/ESBAveragingCounter.cpp
        source/ESBPerformanceCounter.cpp
        source/ESBTimeSeries.cpp
        source/ESBSimplePerformanceCounter.cpp
        source/ESBTime.cpp
        source/ESBSharedAveragingCounter.cpp
        source/ESBSystemConfig.cpp
        source/ESBEmbeddedMapElement.cpp
        source/ESBSharedEmbeddedMap.cpp
        source/ESBEventSocket.cpp
        )
add_library(base STATIC ${SOURCE_FILES})
add_dependencies(base bssl)

target_include_directories(base PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        )

target_include_directories(base PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

#target_compile_options(base PUBLIC ${CPP_FLAGS})

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../unit-tf/include"
        )

set(TEST_LIBS -pthread -ldl unit-tf base bssl_ssl bssl_crypto)

add_unit_test(buddy-allocator-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBBuddyAllocatorTest.cpp)
add_unit_test(buffer-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBBufferTest.cpp)
add_unit_test(discard-allocator-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBDiscardAllocatorTest.cpp)
add_unit_test(fixed-allocator-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBFixedAllocatorTest.cpp)
add_unit_test(list-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBListTest.cpp)
add_unit_test(lockable-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBLockableTest.cpp)
add_unit_test(map-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBMapTest.cpp)
add_unit_test(shared-counter-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSharedIntTest.cpp)
add_unit_test(smart-pointer-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSmartPointerTest.cpp)
add_unit_test(shared-queue-test "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSharedQueueTest.cpp tests/ESBSharedQueueConsumer.cpp tests/ESBSharedQueueProducer.cpp)

add_gtest(shared-embedded-map-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBSharedEmbeddedMapTest.cpp)
add_gtest(averaging-counter-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBAveragingCounterTest.cpp)
add_gtest(simple-performance-counter-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBSimplePerformanceCounterTest.cpp)
add_gtest(time-series-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBTimeSeriesTest.cpp)
add_gtest(discard-allocator-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBDiscardAllocatorTest2.cpp)
add_gtest(logger-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBLoggerTest.cpp)
add_gtest(date-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBDateTest.cpp)
add_gtest(time-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBTimeTest.cpp)
add_gtest(map-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBMapTest2.cpp)
add_gtest(system-config-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBSystemConfigTest.cpp)
add_gtest(socket-address-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBSocketAddressTest.cpp)
add_gtest(connection-pool-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBConnectionPoolTest.cpp)
add_gtest(clear-socket-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBClearSocketTest.cpp)
add_gtest(tls-socket-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBTLSSocketTest.cpp)
add_gtest(signal-handler-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests tests/ESBSignalHandlerTest.cpp)

# Generate certs and private keys

add_custom_command(TARGET tls-socket-gtest POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/create-certs.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
