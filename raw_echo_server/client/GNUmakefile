BASE=../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
-I$(BASE)/raw_echo_server/client

SOURCES=\
AWSClientSocket.cpp\
AWSClientSocketFactory.cpp\
AWSRawEchoClientMain.cpp\
AWSPerformanceCounter.cpp

EXE=$(BASE)/bin/raw_echo_client

STATIC_LIBS =\
$(BASE)/lib/libfoundation
 
all: $(EXE)

include $(BASE)/GNUmakevars
