#ifndef ES_HTTP_ECHO_CLIENT_HANDLER_H
#define ES_HTTP_ECHO_CLIENT_HANDLER_H

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

class HttpEchoClientHandler : public HttpClientHandler {
 public:
  HttpEchoClientHandler(const char *absPath, const char *method,
                        const char *contentType, const unsigned char *body,
                        int bodySize);

  virtual ~HttpEchoClientHandler();

  //
  // ES::HttpClientHandler
  //

  virtual ESB::UInt32 reserveRequestChunk(HttpClientStack &stack,
                                          HttpStream &stream);

  virtual void fillRequestChunk(HttpClientStack &stack, HttpStream &stream,
                                unsigned char *chunk, unsigned int chunkSize);

  virtual Result receiveResponseHeaders(HttpClientStack &stack,
                                        HttpStream &stream);

  virtual ESB::UInt32 reserveResponseChunk(HttpClientStack &stack,
                                           HttpStream &stream);

  virtual void receivePaused(HttpClientStack &stack, HttpStream &stream);

  virtual Result receiveResponseChunk(HttpClientStack &stack,
                                      HttpStream &stream,
                                      unsigned const char *chunk,
                                      ESB::UInt32 chunkSize);

  virtual void endClientTransaction(HttpClientStack &stack, HttpStream &stream,
                                    State state);

 private:
  // Disabled
  HttpEchoClientHandler(const HttpEchoClientHandler &clientHandler);
  void operator=(const HttpEchoClientHandler &clientHandler);

  const char *_absPath;
  const char *_method;
  const char *_contentType;
  const unsigned char *_body;
  const int _bodySize;
  ESB::SharedInt _completedTransactions;
};

}  // namespace ES

#endif
