BASE=../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
-I$(BASE)/sparrowhawk/test_framework

SOURCES=\
ESFAllocatorCleanupHandler.cpp\
ESFBuddyAllocator.cpp\
ESFBuffer.cpp\
ESFBufferPool.cpp\
ESFCommand.cpp\
ESFCommandThread.cpp\
ESFComparator.cpp\
ESFConsoleLogger.cpp\
ESFConnectedTCPSocket.cpp\
ESFCountingSemaphore.cpp\
ESFDate.cpp\
ESFEmbeddedListElement.cpp\
ESFEmbeddedList.cpp\
ESFError.cpp\
ESFFixedAllocator.cpp\
ESFFlag.cpp\
ESFList.cpp\
ESFListeningTCPSocket.cpp\
ESFLockable.cpp\
ESFMap.cpp\
ESFMutex.cpp\
ESFMultiplexedSocket.cpp\
ESFNullLock.cpp\
ESFNullLogger.cpp\
ESFRand.cpp\
ESFReadWriteLock.cpp\
ESFReferenceCount.cpp\
ESFSharedAllocator.cpp\
ESFSharedCounter.cpp\
ESFSharedEmbeddedQueue.cpp\
ESFSharedQueue.cpp\
ESFSmartPointer.cpp\
ESFSmartPointerDebugger.cpp\
ESFSocketMultiplexer.cpp\
ESFSocketMultiplexerDispatcher.cpp\
ESFSocketMultiplexerFactory.cpp\
ESFSystemAllocator.cpp\
ESFThread.cpp\
ESFThreadPool.cpp\
ESFSocketAddress.cpp\
ESFTCPSocket.cpp\
ESFDiscardAllocator.cpp

uname=$(shell echo `uname`)

ifeq ($(uname),Linux)
    SOURCES += ESFEpollMultiplexer.cpp ESFEpollMultiplexerFactory.cpp
endif

STATIC_LIB_PROD_TARGET=libfoundation.a
STATIC_LIB_DEBUG_TARGET=libfoundation_debug.a
DYNAMIC_LIB_PROD_TARGET=libfoundation.so
DYNAMIC_LIB_DEBUG_TARGET=libfoundation_debug.so

all: $(STATIC_LIB_PROD_TARGET) $(STATIC_LIB_DEBUG_TARGET) $(DYNAMIC_LIB_PROD_TARGET) $(DYNAMIC_LIB_DEBUG_TARGET)

include $(BASE)/GNUmakevars
