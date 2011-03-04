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

EXE=$(BASE)/bin/http_echo_client

STATIC_LIBS=$(BASE)/lib/libredux_http_stack \
$(BASE)/lib/libfoundation 

 
all: $(EXE)

include $(BASE)/GNUmakevars

