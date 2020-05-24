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

namespace ES {

class HttpLoadgenHandler : public HttpClientHandler {
 public:
  HttpLoadgenHandler(const char *absPath, const char *method,
                     const char *contentType, const unsigned char *body,
                     int bodySize);

  virtual ~HttpLoadgenHandler();

  //
  // ES::HttpClientHandler
  //

  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer,
                                            HttpClientStream &stream);

  virtual ESB::Error offerRequestChunk(HttpMultiplexer &multiplexer,
                                       HttpClientStream &stream,
                                       ESB::UInt32 *bytesAvailable);

  virtual ESB::Error produceRequestChunk(HttpMultiplexer &multiplexer,
                                         HttpClientStream &stream,
                                         unsigned char *chunk,
                                         ESB::UInt32 bytesRequested);

  virtual ESB::Error consumeResponseChunk(HttpMultiplexer &multiplexer,
                                          HttpClientStream &stream,
                                          const unsigned char *chunk,
                                          ESB::UInt32 chunkSize,
                                          ESB::UInt32 *bytesConsumed);

  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer,
                                HttpClientStream &stream);

  virtual void endTransaction(HttpMultiplexer &multiplexer,
                              HttpClientStream &stream, State state);

 private:
  // Disabled
  HttpLoadgenHandler(const HttpLoadgenHandler &);
  void operator=(const HttpLoadgenHandler &);

  const char *_absPath;
  const char *_method;
  const char *_contentType;
  const unsigned char *_body;
  const int _bodySize;
  ESB::SharedInt _completedTransactions;
};

}  // namespace ES

#endif
