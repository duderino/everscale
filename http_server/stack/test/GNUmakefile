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

EXE_DEBUG_TARGET=stack_test_debug
EXE_PROD_TARGET=stack_test

EXE_DEBUG_DYNAMIC_LIBS =\
$(BASE)/sparrowhawk/foundation/libfoundation_debug.so\
$(BASE)/http_server/stack/libredux_http_parser_debug.so

EXE_PROD_DYNAMIC_LIBS =\
$(BASE)/sparrowhawk/foundation/libfoundation.so\
$(BASE)/http_server/stack/libredux_http_parser.so

EXE_PROD_FLAGS = -Wl,-rpath,/home/y/lib
EXE_DEBUG_FLAGS = -Wl,-rpath,/home/y/lib
 
all: $(EXE_DEBUG_TARGET) $(EXE_PROD_TARGET)

include $(BASE)/GNUmakevars
