BASE=../..

INCLUDE = \
-I$(BASE)/dependencies/sparrowhawk/include\
-I$(BASE)/http_server/stack/formatter\
-I$(BASE)/http_server/stack/parser\
-I$(BASE)/http_server/stack/message\
-I$(BASE)/http_server/stack/common\
-I$(BASE)/http_server/stack/api\
-I$(BASE)/http_server/stack/server\
-I$(BASE)/http_server/stack/client\
-I$(BASE)/http_server/stack/stack\
-I$(BASE)/http_server/stack/counters\
-I$(BASE)/http_server/stack/resolver

VPATH = \
$(BASE)/http_server/stack/formatter\
$(BASE)/http_server/stack/parser\
$(BASE)/http_server/stack/message\
$(BASE)/http_server/stack/common\
$(BASE)/http_server/stack/api\
$(BASE)/http_server/stack/server\
$(BASE)/http_server/stack/client\
$(BASE)/http_server/stack/stack\
$(BASE)/http_server/stack/counters\
$(BASE)/http_server/stack/resolver

SOURCES=\
AWSHttpResolver.cpp\
AWSHttpDefaultResolver.cpp\
AWSHttpResponse.cpp\
AWSHttpRequestUri.cpp\
AWSHttpRequest.cpp\
AWSHttpMessage.cpp\
AWSHttpHeader.cpp\
AWSHttpServerSocketFactory.cpp\
AWSHttpListeningSocket.cpp\
AWSHttpServerHandler.cpp\
AWSHttpServerSocket.cpp\
AWSHttpServerTransaction.cpp\
AWSHttpTransaction.cpp\
AWSHttpStack.cpp\
AWSHttpRequestParser.cpp\
AWSHttpMessageParser.cpp\
AWSHttpRequestUriParser.cpp\
AWSHttpResponseParser.cpp\
AWSHttpClientTransactionFactory.cpp\
AWSHttpClientTransaction.cpp\
AWSHttpClientHandler.cpp\
AWSHttpResponseFormatter.cpp\
AWSHttpMessageFormatter.cpp\
AWSHttpRequestUriFormatter.cpp\
AWSHttpRequestFormatter.cpp\
AWSHttpUtil.cpp\
AWSPerformanceCounter.cpp\
AWSHistoricalPerformanceCounter.cpp\
AWSSimplePerformanceCounter.cpp\
AWSHttpConnectionPool.cpp\
AWSHttpClientSocketFactory.cpp\
AWSHttpClientSocket.cpp\
AWSAveragingCounter.cpp\
AWSHttpServerCounters.cpp\
AWSHttpServerSimpleCounters.cpp\
AWSHttpClientCounters.cpp\
AWSHttpClientSimpleCounters.cpp\
AWSHttpClientHistoricalCounters.cpp

DYNAMIC_LIBS =\
$(BASE)/dependencies/sparrowhawk/lib/libsparrowhawk

LIB=$(BASE)/lib/libredux_http_stack

all: $(LIB)

test:

include $(BASE)/GNUmakevars
