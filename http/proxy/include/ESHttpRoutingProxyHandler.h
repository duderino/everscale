#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#define ES_HTTP_ROUTING_PROXY_HANDLER_H

#ifndef ES_HTTP_PROXY_HANDLER_H
#include <ESHttpProxyHandler.h>
#endif

#ifndef ES_HTTP_ROUTER_H
#include <ESHttpRouter.h>
#endif

namespace ES {

class HttpRoutingProxyHandler : public HttpProxyHandler {
 public:
  HttpRoutingProxyHandler(HttpRouter &router);

  virtual ~HttpRoutingProxyHandler();

  //
  // ES:HttpServerHandler via ES::HttpProxyHandler
  //

  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer,
                                      ESB::SocketAddress *address);
  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer,
                                      HttpServerStream &stream);
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer,
                                           HttpServerStream &stream);
  virtual ESB::Error consumeRequestChunk(HttpMultiplexer &multiplexer,
                                         HttpServerStream &stream,
                                         unsigned const char *chunk,
                                         ESB::UInt32 chunkSize,
                                         ESB::UInt32 *bytesConsumed);
  virtual ESB::Error offerResponseChunk(HttpMultiplexer &multiplexer,
                                        HttpServerStream &stream,
                                        ESB::UInt32 *bytesAvailable);
  virtual ESB::Error produceResponseChunk(HttpMultiplexer &multiplexer,
                                          HttpServerStream &stream,
                                          unsigned char *chunk,
                                          ESB::UInt32 bytesRequested);
  virtual void endServerTransaction(HttpMultiplexer &multiplexer,
                                    HttpServerStream &stream,
                                    HttpServerHandler::State state);

  //
  // ES::HttpClientHandler via ES:HttpProxyHandler
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
  virtual void endClientTransaction(HttpMultiplexer &multiplexer,
                                    HttpClientStream &stream,
                                    HttpClientHandler::State state);

 private:
  // Disabled
  HttpRoutingProxyHandler(const HttpRoutingProxyHandler &);
  void operator=(const HttpRoutingProxyHandler &);

  ESB::Error sendResponse(HttpServerStream &stream, int statusCode,
                          const char *reasonPhrase);

  HttpRouter &_router;
};

}  // namespace ES

#endif
