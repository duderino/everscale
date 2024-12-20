#ifndef ES_HTTP_ORIGIN_HANDLER_H
#define ES_HTTP_ORIGIN_HANDLER_H

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

#ifndef ES_HTTP_TEST_PARAMS_H
#include <ESHttpTestParams.h>
#endif

#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

namespace ES {

class HttpOriginHandler : public HttpServerHandler {
 public:
  HttpOriginHandler(const HttpTestParams &params);

  virtual ~HttpOriginHandler();

  //
  // ES::HttpServerHandler
  //

  virtual ESB::Error acceptConnection(HttpMultiplexer &stack, ESB::SocketAddress *address);
  virtual ESB::Error beginTransaction(HttpMultiplexer &stack, HttpServerStream &stream);
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &stack, HttpServerStream &stream);
  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                        unsigned const char *chunk, ESB::UInt64 chunkSize, ESB::UInt64 *bytesConsumed);
  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                       ESB::UInt64 *bytesAvailable);
  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream, unsigned char *chunk,
                                         ESB::UInt64 bytesRequested);
  virtual void endTransaction(HttpMultiplexer &stack, HttpServerStream &stream, State state);
  virtual void dumpServerCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const;

 private:
  const HttpTestParams &_params;
  HttpServerSimpleCounters _serverCounters;

  ESB_DEFAULT_FUNCS(HttpOriginHandler);
};

}  // namespace ES

#endif
