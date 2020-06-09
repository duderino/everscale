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

  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address);
  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream);
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer, HttpServerStream &serverStream);
  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                        unsigned const char *body, ESB::UInt32 bytesOffered,
                                        ESB::UInt32 *bytesConsumed);
  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                       ESB::UInt32 *bytesAvailable);
  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                         unsigned char *body, ESB::UInt32 bytesRequested);
  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                              HttpServerHandler::State state);

  //
  // ES::HttpClientHandler via ES:HttpProxyHandler
  //

  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &clientStream);
  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                      ESB::UInt32 *bytesAvailable);
  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                        unsigned char *body, ESB::UInt32 bytesRequested);
  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                         const unsigned char *body, ESB::UInt32 bytesOffered,
                                         ESB::UInt32 *bytesConsumed);
  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                              HttpClientHandler::State state);
  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream);

 private:
  // Disabled
  HttpRoutingProxyHandler(const HttpRoutingProxyHandler &);
  void operator=(const HttpRoutingProxyHandler &);

  HttpRouter &_router;
};

}  // namespace ES

#endif
