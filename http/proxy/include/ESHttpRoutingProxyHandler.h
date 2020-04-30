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

  virtual HttpServerHandler::Result acceptConnection(
      HttpServerStack &stack, ESB::SocketAddress *address);
  virtual HttpServerHandler::Result beginServerTransaction(
      HttpServerStack &stack, HttpStream &stream);
  virtual HttpServerHandler::Result receiveRequestHeaders(
      HttpServerStack &stack, HttpStream &stream);
  virtual ESB::UInt32 reserveRequestChunk(HttpServerStack &stack,
                                          HttpStream &stream);
  virtual HttpServerHandler::Result receiveRequestChunk(
      HttpServerStack &stack, HttpStream &stream, unsigned const char *chunk,
      ESB::UInt32 chunkSize);
  virtual void receivePaused(HttpServerStack &stack, HttpStream &stream);
  virtual ESB::UInt32 reserveResponseChunk(HttpServerStack &stack,
                                           HttpStream &stream);
  virtual void fillResponseChunk(HttpServerStack &stack, HttpStream &stream,
                                 unsigned char *chunk, ESB::UInt32 chunkSize);
  virtual void endServerTransaction(HttpServerStack &stack, HttpStream &stream,
                                    HttpServerHandler::State state);

  //
  // ES::HttpClientHandler via ES:HttpProxyHandler
  //

  virtual ESB::UInt32 reserveRequestChunk(HttpClientStack &stack,
                                          HttpStream &stream);
  virtual void fillRequestChunk(HttpClientStack &stack, HttpStream &stream,
                                unsigned char *chunk, ESB::UInt32 chunkSize);
  virtual HttpClientHandler::Result receiveResponseHeaders(
      HttpClientStack &stack, HttpStream &stream);
  virtual ESB::UInt32 reserveResponseChunk(HttpClientStack &stack,
                                           HttpStream &stream);
  virtual void receivePaused(HttpClientStack &stack, HttpStream &stream);
  virtual HttpClientHandler::Result receiveResponseChunk(
      HttpClientStack &stack, HttpStream &stream, unsigned const char *chunk,
      ESB::UInt32 chunkSize);
  virtual void endClientTransaction(HttpClientStack &stack, HttpStream &stream,
                                    HttpClientHandler::State state);

 private:
  // Disabled
  HttpRoutingProxyHandler(const HttpRoutingProxyHandler &);
  void operator=(const HttpRoutingProxyHandler &);

  HttpServerHandler::Result sendResponse(HttpStream &stream, int statusCode,
                                         const char *reasonPhrase);

  HttpRouter &_router;
};

}  // namespace ES

#endif
