#ifndef ES_HTTP_ROUTING_PROXY_HANDLER_H
#define ES_HTTP_ROUTING_PROXY_HANDLER_H

#ifndef ES_HTTP_PROXY_HANDLER_H
#include <ESHttpProxyHandler.h>
#endif

#ifndef ES_HTTP_ROUTER_H
#include <ESHttpRouter.h>
#endif

#ifndef ES_HTTP_SERVER_SIMPLE_COUNTERS_H
#include <ESHttpServerSimpleCounters.h>
#endif

#ifndef ES_HTTP_CLIENT_SIMPLE_COUNTERS_H
#include <ESHttpClientSimpleCounters.h>
#endif

namespace ES {

class HttpRoutingProxyHandler : public HttpProxyHandler {
 public:
  HttpRoutingProxyHandler(HttpRouter &router, ESB::Allocator &allocator);

  virtual ~HttpRoutingProxyHandler();

  //
  // ES:HttpServerHandler via ES::HttpProxyHandler
  //

  virtual ESB::Error acceptConnection(HttpMultiplexer &multiplexer, ESB::SocketAddress *address);
  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream);
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &multiplexer, HttpServerStream &serverStream);
  virtual ESB::Error consumeRequestBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                        unsigned const char *body, ESB::UInt64 bytesOffered,
                                        ESB::UInt64 *bytesConsumed);
  virtual ESB::Error offerResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                       ESB::UInt64 *bytesAvailable);
  virtual ESB::Error produceResponseBody(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                                         unsigned char *body, ESB::UInt64 bytesRequested);
  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpServerStream &serverStream,
                              HttpServerHandler::State state);

  //
  // ES::HttpClientHandler via ES:HttpProxyHandler
  //

  virtual ESB::Error beginTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream);
  virtual ESB::Error receiveResponseHeaders(HttpMultiplexer &multiplexer, HttpClientStream &clientStream);
  virtual ESB::Error offerRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                      ESB::UInt64 *bytesAvailable);
  virtual ESB::Error produceRequestBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                        unsigned char *body, ESB::UInt64 bytesRequested);
  virtual ESB::Error consumeResponseBody(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                                         const unsigned char *body, ESB::UInt64 bytesOffered,
                                         ESB::UInt64 *bytesConsumed);
  virtual void endTransaction(HttpMultiplexer &multiplexer, HttpClientStream &clientStream,
                              HttpClientHandler::State state);
  virtual ESB::Error endRequest(HttpMultiplexer &multiplexer, HttpClientStream &clientStream);
  virtual void dumpClientCounters(ESB::Logger &logger, ESB::Logger::Severity severity) const;

 private:
  ESB::Error onClientRecvBlocked(HttpServerStream &serverStream, HttpClientStream &clientStream);
  ESB::Error onServerRecvBlocked(HttpServerStream &serverStream, HttpClientStream &clientStream);
  ESB::Error onClientSendBlocked(HttpServerStream &serverStream, HttpClientStream &clientStream);
  ESB::Error onServerSendBlocked(HttpServerStream &serverStream, HttpClientStream &clientStream);

  HttpRouter &_router;
  HttpServerSimpleCounters _serverCounters;
  HttpClientSimpleCounters _clientCounters;

  ESB_DEFAULT_FUNCS(HttpRoutingProxyHandler);
};

}  // namespace ES

#endif
