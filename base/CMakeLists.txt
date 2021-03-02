cmake_minimum_required(VERSION 3.5)
project(base VERSION ${VERSION} LANGUAGES CXX)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

set(SOURCE_FILES
        source/ESBAllocator.cpp
        source/ESBAveragingCounter.cpp
        source/ESBBuddyAllocator.cpp
        source/ESBBuffer.cpp
        source/ESBBufferPool.cpp
        source/ESBClearSocket.cpp
        source/ESBClientTLSSocket.cpp
        source/ESBCommand.cpp
        source/ESBCommandThread.cpp
	source/ESBCompactStringMap.cpp
        source/ESBComparator.cpp
        source/ESBConnectedSocket.cpp
        source/ESBConnectionPool.cpp
        source/ESBCountingSemaphore.cpp
        source/ESBDate.cpp
        source/ESBTimer.cpp
        source/ESBDiscardAllocator.cpp
        source/ESBDnsClient.cpp
        source/ESBEmbeddedList.cpp
        source/ESBEmbeddedListElement.cpp
        source/ESBEmbeddedMapElement.cpp
        source/ESBEpollMultiplexer.cpp
        source/ESBError.cpp
        source/ESBEventSocket.cpp
        source/ESBFixedAllocator.cpp
        source/ESBFlatTimingWheel.cpp
        source/ESBHostAddress.cpp
        source/ESBList.cpp
        source/ESBListeningSocket.cpp
        source/ESBLockable.cpp
        source/ESBLogger.cpp
        source/ESBMap.cpp
        source/ESBMultiplexedSocket.cpp
        source/ESBMutex.cpp
        source/ESBNullLock.cpp
        source/ESBNullLogger.cpp
        source/ESBPerformanceCounter.cpp
        source/ESBRand.cpp
        source/ESBReadWriteLock.cpp
        source/ESBReferenceCount.cpp
        source/ESBServerTLSSocket.cpp
        source/ESBSharedAllocator.cpp
        source/ESBSharedAveragingCounter.cpp
        source/ESBSharedEmbeddedMap.cpp
        source/ESBSharedEmbeddedQueue.cpp
        source/ESBSharedInt.cpp
        source/ESBSharedQueue.cpp
        source/ESBSignalHandler.cpp
        source/ESBSimpleFileLogger.cpp
        source/ESBSimplePerformanceCounter.cpp
        source/ESBSmartPointer.cpp
        source/ESBSmartPointerDebugger.cpp
        source/ESBSocketAddress.cpp
        source/ESBSocket.cpp
        source/ESBSocketMultiplexer.cpp
        source/ESBString.cpp
        source/ESBSystemAllocator.cpp
        source/ESBSystemConfig.cpp
        source/ESBSystemDnsClient.cpp
        source/ESBSystemTimeSource.cpp
        source/ESBThread.cpp
        source/ESBThreadPool.cpp
        source/ESBTime.cpp
        source/ESBTimeSeries.cpp
        source/ESBTimeSourceCache.cpp
        source/ESBTimeSource.cpp
        source/ESBTLSSocket.cpp
        )

add_library(base STATIC ${SOURCE_FILES})
add_dependencies(base bssl)

target_include_directories(base PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        )

target_include_directories(base PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

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

add_gtest(shared-embedded-map-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSharedEmbeddedMapTest.cpp)
add_gtest(averaging-counter-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBAveragingCounterTest.cpp)
add_gtest(simple-performance-counter-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSimplePerformanceCounterTest.cpp)
add_gtest(time-series-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBTimeSeriesTest.cpp)
add_gtest(discard-allocator-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBDiscardAllocatorTest2.cpp)
add_gtest(logger-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBLoggerTest.cpp)
add_gtest(date-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBDateTest.cpp)
add_gtest(time-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBTimeTest.cpp)
add_gtest(map-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBMapTest2.cpp)
add_gtest(system-config-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSystemConfigTest.cpp)
add_gtest(socket-address-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSocketAddressTest.cpp)
add_gtest(connection-pool-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBConnectionPoolTest.cpp)
add_gtest(clear-socket-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBClearSocketTest.cpp)
add_gtest(tls-socket-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBTLSSocketTest.cpp)
add_gtest(signal-handler-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSignalHandlerTest.cpp)
add_gtest(flat-timing-wheel-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBFlatTimingWheelTest.cpp)
add_gtest(compact-string-map-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBCompactStringMapTest.cpp)
add_gtest(string-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBStringTest.cpp)
add_gtest(smart-pointer-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESBSmartPointerTest2.cpp)

# Generate certs and private keys

add_custom_command(TARGET tls-socket-gtest POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/create-certs.sh
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
