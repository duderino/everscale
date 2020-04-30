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

  virtual Result acceptConnection(HttpServerStack &stack,
                                  ESB::SocketAddress *address);
  virtual Result beginServerTransaction(HttpServerStack &stack,
                                        HttpStream &stream);
  virtual Result receiveRequestHeaders(HttpServerStack &stack,
                                       HttpStream &stream);
  virtual ESB::UInt32 reserveRequestChunk(HttpServerStack &stack,
                                          HttpStream &stream);
  virtual Result receiveRequestChunk(HttpServerStack &stack, HttpStream &stream,
                                     unsigned const char *chunk,
                                     ESB::UInt32 chunkSize);
  virtual void receivePaused(HttpServerStack &stack, HttpStream &stream);
  virtual ESB::UInt32 reserveResponseChunk(HttpServerStack &stack,
                                           HttpStream &stream);
  virtual void fillResponseChunk(HttpServerStack &stack, HttpStream &stream,
                                 unsigned char *chunk, ESB::UInt32 chunkSize);
  virtual void endServerTransaction(HttpServerStack &stack, HttpStream &stream,
                                    State state);

 private:
  // Disabled
  HttpOriginHandler(const HttpOriginHandler &);
  void operator=(const HttpOriginHandler &);
};

}  // namespace ES

#endif
