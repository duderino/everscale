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
  virtual Result beginServerTransaction(HttpMultiplexer &stack,
                                        HttpStream &stream);
  virtual Result receiveRequestHeaders(HttpMultiplexer &stack,
                                       HttpStream &stream);
  virtual ESB::UInt32 reserveRequestChunk(HttpMultiplexer &stack,
                                          HttpStream &stream);
  virtual Result receiveRequestChunk(HttpMultiplexer &stack, HttpStream &stream,
                                     unsigned const char *chunk,
                                     ESB::UInt32 chunkSize);
  virtual void receivePaused(HttpMultiplexer &stack, HttpStream &stream);
  virtual ESB::UInt32 reserveResponseChunk(HttpMultiplexer &stack,
                                           HttpStream &stream);
  virtual void fillResponseChunk(HttpMultiplexer &stack, HttpStream &stream,
                                 unsigned char *chunk, ESB::UInt32 chunkSize);
  virtual void endServerTransaction(HttpMultiplexer &stack, HttpStream &stream,
                                    State state);

 private:
  // Disabled
  HttpOriginHandler(const HttpOriginHandler &);
  void operator=(const HttpOriginHandler &);
};

}  // namespace ES

#endif
