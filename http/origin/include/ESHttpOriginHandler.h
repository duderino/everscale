#ifndef ES_HTTP_ORIGIN_HANDLER_H
#define ES_HTTP_ORIGIN_HANDLER_H

#ifndef ES_HTTP_SERVER_HANDLER_H
#include <ESHttpServerHandler.h>
#endif

namespace ES {

class HttpOriginHandler : public HttpServerHandler {
 public:
  HttpOriginHandler();

  virtual ~HttpOriginHandler();

  //
  // ES::HttpServerHandler
  //

  virtual Result acceptConnection(HttpMultiplexer &stack,
                                  ESB::SocketAddress *address);
  virtual Result beginTransaction(HttpMultiplexer &stack,
                                  HttpServerStream &stream);
  virtual Result receiveRequestHeaders(HttpMultiplexer &stack,
                                       HttpServerStream &stream);
  virtual ESB::UInt32 reserveRequestChunk(HttpMultiplexer &stack,
                                          HttpServerStream &stream);
  virtual Result receiveRequestChunk(HttpMultiplexer &stack,
                                     HttpServerStream &stream,
                                     unsigned const char *chunk,
                                     ESB::UInt32 chunkSize);
  virtual void receivePaused(HttpMultiplexer &stack, HttpServerStream &stream);
  virtual ESB::UInt32 reserveResponseChunk(HttpMultiplexer &stack,
                                           HttpServerStream &stream);
  virtual void fillResponseChunk(HttpMultiplexer &stack,
                                 HttpServerStream &stream, unsigned char *chunk,
                                 ESB::UInt32 chunkSize);
  virtual void endTransaction(HttpMultiplexer &stack, HttpServerStream &stream,
                              State state);

 private:
  // Disabled
  HttpOriginHandler(const HttpOriginHandler &);
  void operator=(const HttpOriginHandler &);
};

}  // namespace ES

#endif
