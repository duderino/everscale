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

EXE=raw_echo_server

STATIC_LIBS =\
$(BASE)/sparrowhawk/foundation/libfoundation
 
all: $(EXE)

include $(BASE)/GNUmakevars
