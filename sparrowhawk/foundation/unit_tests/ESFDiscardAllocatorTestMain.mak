BASE=../../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
-I$(BASE)/sparrowhawk/test_framework\
-I$(BASE)/sparrowhawk/foundation/unit_tests

SOURCES=\
ESFDiscardAllocatorTest.cpp \
ESFDiscardAllocatorTestMain.cpp

STATIC_LIBS =\
$(BASE)/sparrowhawk/foundation/libfoundation \
$(BASE)/sparrowhawk/test_framework/libtest_framework

DEBUG_ONLY=1
EXE=discard_allocator_test

all: $(EXE)

run:
	@echo "32 bit test"
	./$(EXE)_debug_32
	@echo "64 bit test"
	./$(EXE)_debug_64

include $(BASE)/GNUmakevars
