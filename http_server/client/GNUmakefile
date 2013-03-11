BASE=../..

INCLUDE = \
-I$(BASE)/dependencies/sparrowhawk/include\
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

DYNAMIC_LIBS =\
$(BASE)/lib/libredux_http_stack\
$(BASE)/dependencies/sparrowhawk/lib/libsparrowhawk

 
all: $(EXE)

test:

include $(BASE)/GNUmakevars

