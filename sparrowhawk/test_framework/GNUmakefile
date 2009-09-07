BASE=../..

INCLUDE = \
-I$(BASE)/sparrowhawk/test_framework

SOURCES=\
ESTFDate.cpp\
ESTFConcurrencyDecorator.cpp\
ESTFConcurrencyComposite.cpp\
ESTFComponentThread.cpp\
ESTFComposite.cpp\
ESTFRandomizedDecorator.cpp\
ESTFRepetitionDecorator.cpp\
ESTFResultCollector.cpp\
ESTFThread.cpp

STATIC_LIB_PROD_TARGET=libtest_framework.a
STATIC_LIB_DEBUG_TARGET=libtest_framework_debug.a
DYNAMIC_LIB_PROD_TARGET=libtest_framework.so
DYNAMIC_LIB_DEBUG_TARGET=libtest_framework_debug.so

all: $(STATIC_LIB_PROD_TARGET) $(STATIC_LIB_DEBUG_TARGET) $(DYNAMIC_LIB_PROD_TARGET) $(DYNAMIC_LIB_DEBUG_TARGET)

include $(BASE)/GNUmakevars
