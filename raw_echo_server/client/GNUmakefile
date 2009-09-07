BASE=../..

INCLUDE = \
-I$(BASE)/sparrowhawk/foundation\
-I$(BASE)/raw_echo_server/client\
-I$(BASE)/raw_echo_server/common

VPATH = \
$(BASE)/raw_echo_server/client\
$(BASE)/raw_echo_server/common

SOURCES=\
AWSClientSocket.cpp\
AWSClientSocketFactory.cpp\
AWSRawEchoClientMain.cpp\
AWSPerformanceCounter.cpp

EXE_DEBUG_TARGET=raw_echo_client_debug
EXE_PROD_TARGET=raw_echo_client

EXE_DEBUG_DYNAMIC_LIB_DEPENDENCIES =\
$(BASE)/sparrowhawk/foundation/libfoundation_debug.so

EXE_PROD_DYNAMIC_LIB_DEPENDENCIES =\
$(BASE)/sparrowhawk/foundation/libfoundation.so

DYNAMIC_LIB_PATH = -L $(BASE)/sparrowhawk/foundation 

EXE_PROD_DYNAMIC_LIBS = -lfoundation
EXE_DEBUG_DYNAMIC_LIBS = -lfoundation_debug

EXE_PROD_FLAGS = -Wl,-rpath,/home/y/lib
EXE_DEBUG_FLAGS = -Wl,-rpath,/home/y/lib
 
all: $(EXE_DEBUG_TARGET) $(EXE_PROD_TARGET)

include $(BASE)/GNUmakevars
