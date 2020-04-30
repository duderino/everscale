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
  virtual ESB::Error beginServerTransaction(HttpMultiplexer &multiplexer,
                                            HttpServerStream &stream);
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer,
                                           HttpServerStream &stream);
  virtual ESB::UInt32 reserveRequestChunk(HttpMultiplexer &multiplexer,
                                          HttpServerStream &stream);
  virtual ESB::Error receiveRequestChunk(HttpMultiplexer &multiplexer,
                                         HttpServerStream &stream,
                                         unsigned const char *chunk,
                                         ESB::UInt32 chunkSize);
  virtual void receivePaused(HttpMultiplexer &multiplexer,
                             HttpServerStream &stream);
  virtual ESB::UInt32 reserveResponseChunk(HttpMultiplexer &multiplexer,
                                           HttpServerStream &stream);
  virtual ESB::Error fillResponseChunk(HttpMultiplexer &multiplexer,
                                       HttpServerStream &stream,
                                       unsigned char *chunk,
                                       ESB::UInt32 chunkSize);
  virtual void endServerTransaction(HttpMultiplexer &multiplexer,
                                    HttpServerStream &stream,
                                    HttpServerHandler::State state);

  //
  // ES::HttpClientHandler via ES:HttpProxyHandler
  //

  virtual ESB::UInt32 reserveRequestChunk(HttpMultiplexer &multiplexer,
                                          HttpClientStream &stream);
  virtual ESB::Error fillRequestChunk(HttpMultiplexer &multiplexer,
                                      HttpClientStream &stream,
                                      unsigned char *chunk,
                                      ESB::UInt32 chunkSize);
  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer,
                                            HttpClientStream &stream);
  virtual ESB::UInt32 reserveResponseChunk(HttpMultiplexer &multiplexer,
                                           HttpClientStream &stream);
  virtual void receivePaused(HttpMultiplexer &multiplexer,
                             HttpClientStream &stream);
  virtual ESB::Error receiveResponseChunk(HttpMultiplexer &multiplexer,
                                          HttpClientStream &stream,
                                          unsigned const char *chunk,
                                          ESB::UInt32 chunkSize);
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
