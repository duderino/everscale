BASE=../..

INCLUDE = \
-I$(BASE)/dependencies/sparrowhawk/include\
-I$(BASE)/raw_echo_server/client

SOURCES=\
AWSClientSocket.cpp\
AWSClientSocketFactory.cpp\
AWSRawEchoClientMain.cpp\
AWSPerformanceCounter.cpp

EXE=$(BASE)/bin/raw_echo_client

STATIC_LIBS =\
$(BASE)/dependencies/sparrowhawk/lib/libsparrowhawk
 
all: $(EXE)

include $(BASE)/GNUmakevars
