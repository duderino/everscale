#ifndef ES_HTTP_ORIGIN_HANDLER_H
#define ES_HTTP_ORIGIN_HANDLER_H

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

namespace ES {

class HttpOriginHandler : public HttpServerHandler {
 public:
  HttpOriginHandler(const char *contentType, const unsigned char *responseBody, ESB::UInt32 responseSize,
                    ESB::Int64 requestSize);

  virtual ~HttpOriginHandler();

  //
  // ES::HttpServerHandler
  //

  virtual ESB::Error acceptConnection(HttpMultiplexer &stack, ESB::SocketAddress *address);
  virtual ESB::Error beginTransaction(HttpMultiplexer &stack, HttpServerStream &stream);
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &stack, HttpServerStream &stream);
  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                        unsigned const char *chunk, ESB::UInt32 chunkSize, ESB::UInt32 *bytesConsumed);
  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream,
                                       ESB::UInt32 *bytesAvailable);
  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &stream, unsigned char *chunk,
                                         ESB::UInt32 bytesRequested);
  virtual void endTransaction(HttpMultiplexer &stack, HttpServerStream &stream, State state);

 private:
  // Disabled
  HttpOriginHandler(const HttpOriginHandler &);
  void operator=(const HttpOriginHandler &);

  const char *_contentType;
  const unsigned char *_responseBody;
  const ESB::UInt32 _responseSize;
  const ESB::Int64 _requestSize;
};

}  // namespace ES

#endif
