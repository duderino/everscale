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

LIB = $(BASE)/lib/libfoundation

all: $(LIB) 

include $(BASE)/GNUmakevars
