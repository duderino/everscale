BASE=../../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
-I$(BASE)/sparrowhawk/test_framework\
-I$(BASE)/sparrowhawk/foundation/unit_tests

SOURCES=\
ESFLockableTest.cpp \
ESFLockableTestMain.cpp

STATIC_LIBS =\
$(BASE)/lib/libfoundation \
$(BASE)/lib/libtest_framework

DEBUG_ONLY=1
EXE=lockable_test

all: $(EXE)

run:
	@echo "32 bit test"
	./$(EXE)_debug_32
	@echo "64 bit test"
	./$(EXE)_debug_64

include $(BASE)/GNUmakevars
