BASE=../..

INCLUDE = \
-I$(BASE)/dependencies/sparrowhawk/include\
-I$(BASE)/raw_echo_server/server\
-I$(BASE)/raw_echo_server/common

VPATH = \
$(BASE)/raw_echo_server/server\
$(BASE)/raw_echo_server/common

SOURCES=\
AWSListeningSocket.cpp\
AWSRawEchoServerMain.cpp\
AWSServerSocket.cpp

EXE=$(BASE)/bin/raw_echo_server

STATIC_LIBS =\
$(BASE)/dependencies/sparrowhawk/lib/libsparrowhawk
 
all: $(EXE)

test:

include $(BASE)/GNUmakevars
