BASE=../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
-I$(BASE)/http_server/stack/formatter\
-I$(BASE)/http_server/stack/parser\
-I$(BASE)/http_server/stack/message\
-I$(BASE)/http_server/stack/common\
-I$(BASE)/http_server/client\
-I$(BASE)/http_server/stack/common\
-I$(BASE)/http_server/stack/api\
-I$(BASE)/http_server/stack/client\
-I$(BASE)/http_server/stack/server\
-I$(BASE)/http_server/stack/stack\
-I$(BASE)/http_server/stack/counters\
-I$(BASE)/http_server/stack/resolver

VPATH = \
$(BASE)/http_server/server

SOURCES=\
AWSHttpEchoClientContext.cpp\
AWSHttpEchoClientHandler.cpp\
AWSHttpEchoClientMain.cpp\
AWSHttpEchoClientRequestBuilder.cpp

EXE_DEBUG_TARGET=http_echo_client_debug
EXE_PROD_TARGET=http_echo_client

EXE_DEBUG_DYNAMIC_LIB_DEPENDENCIES =\
$(BASE)/sparrowhawk/foundation/libfoundation_debug.so\
$(BASE)/http_server/stack/libredux_http_parser_debug.so

EXE_PROD_DYNAMIC_LIB_DEPENDENCIES =\
$(BASE)/sparrowhawk/foundation/libfoundation.so\
$(BASE)/http_server/stack/libredux_http_parser.so

DYNAMIC_LIB_PATH = -L $(BASE)/sparrowhawk/foundation -L $(BASE)/http_server/stack

EXE_PROD_DYNAMIC_LIBS = -lfoundation -lredux_http_parser
EXE_DEBUG_DYNAMIC_LIBS = -lfoundation_debug -lredux_http_parser_debug

EXE_PROD_FLAGS = -Wl,-rpath,/home/y/lib
EXE_DEBUG_FLAGS = -Wl,-rpath,/home/y/lib
 
all: $(EXE_DEBUG_TARGET) $(EXE_PROD_TARGET)

include $(BASE)/GNUmakevars

