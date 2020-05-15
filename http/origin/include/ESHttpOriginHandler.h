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

  virtual ESB::Error acceptConnection(HttpMultiplexer &stack,
                                      ESB::SocketAddress *address);
  virtual ESB::Error beginTransaction(HttpMultiplexer &stack,
                                      HttpServerStream &stream);
  virtual ESB::Error receiveRequestHeaders(HttpMultiplexer &stack,
                                           HttpServerStream &stream);
  virtual ESB::Error requestChunkCapacity(HttpMultiplexer &multiplexer,
                                          HttpServerStream &stream,
                                          ESB::UInt32 *maxChunkSize);
  virtual ESB::Error receiveRequestChunk(HttpMultiplexer &stack,
                                         HttpServerStream &stream,
                                         unsigned const char *chunk,
                                         ESB::UInt32 chunkSize);
  virtual ESB::Error offerResponseChunk(HttpMultiplexer &multiplexer,
                                        HttpServerStream &stream,
                                        ESB::UInt32 *maxChunkSize);
  virtual ESB::Error takeResponseChunk(HttpMultiplexer &stack,
                                       HttpServerStream &stream,
                                       unsigned char *chunk,
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
