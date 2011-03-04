BASE=../../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
-I$(BASE)/http_server/stack/formatter\
-I$(BASE)/http_server/stack/parser\
-I$(BASE)/http_server/stack/message\
-I$(BASE)/http_server/stack/common\
-I$(BASE)/http_server/stack/test

SOURCES=\
AWSHttpStackTest.cpp

EXE=stack_test

DYNAMIC_LIBS =\
$(BASE)/lib/libfoundation \
$(BASE)/lib/libredux_http_stack

all: $(EXE)

include $(BASE)/GNUmakevars
