BASE=../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
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
$(BASE)/lib/libfoundation
 
all: $(EXE)

include $(BASE)/GNUmakevars
