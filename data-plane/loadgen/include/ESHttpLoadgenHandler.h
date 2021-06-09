#ifndef ES_HTTP_LOADGEN_HANDLER_H
#define ES_HTTP_LOADGEN_HANDLER_H

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ES_HTTP_CLIENT_HANDLER_H
#include <ESHttpClientHandler.h>
#endif

#ifndef ES_HTTP_CONNECTION_POOL_H
#include <ESHttpConnectionPool.h>
#endif

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

namespace ES {

class HttpLoadgenHandler : public HttpClientHandler {
 public:
  HttpLoadgenHandler(const HttpTestParams &params);

  virtual ~HttpLoadgenHandler();

  //
  // ES::HttpClientHandler
  //

  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream);

  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &stream);

  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                      ESB::UInt64 *bytesAvailable);

  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &stream, unsigned char *chunk,
                                        ESB::UInt64 bytesRequested);

  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &stream,
                                         const unsigned char *chunk, ESB::UInt64 chunkSize, ESB::UInt64 *bytesConsumed);

  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &stream);

  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &stream, State state);

 private:
  const HttpTestParams &_params;
  ESB::SharedInt _completedTransactions;

  ESB_DEFAULT_FUNCS(HttpLoadgenHandler);
};

}  // namespace ES

#endif
