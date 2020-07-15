#ifndef ES_HTTP_PROXY_CONTEXT_H
#define ES_HTTP_PROXY_CONTEXT_H

#ifndef ES_HTTP_SERVER_STREAM_H
#include <ESHttpServerStream.h>
#endif

#ifndef ES_HTTP_CLIENT_STREAM_H
#include <ESHttpClientStream.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ES {

class HttpRoutingProxyContext {
 public:
  HttpRoutingProxyContext();

  virtual ~HttpRoutingProxyContext();

  inline HttpServerStream *serverStream() { return _serverStream; }
  inline HttpClientStream *clientStream() { return _clientStream; }

  inline void setServerStream(HttpServerStream *serverStream) {
    assert(serverStream);
    _serverStream = serverStream;
  }

  inline void setClientStream(HttpClientStream *clientStream) {
    assert(clientStream);
    _clientStream = clientStream;
  }

  inline ESB::UInt32 clientStreamResponseOffset() const { return _clientStreamResponseOffset; }

  inline void setClientStreamResponseOffset(ESB::UInt32 clientStreamResponseOffset) {
    _clientStreamResponseOffset = clientStreamResponseOffset;
  }

  inline ESB::UInt32 serverStreamRequestOffset() const { return _serverStreamRequestOffset; }

  inline void setServerStreamRequestOffset(ESB::UInt32 serverStreamRequestOffset) {
    _serverStreamRequestOffset = serverStreamRequestOffset;
  }

  inline bool receivedOutboundResponse() const { return _receivedOutboundResponse; }

  inline void setReceivedOutboundResponse(bool receivedOutboundResponse) {
    _receivedOutboundResponse = receivedOutboundResponse;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  HttpRoutingProxyContext(const HttpRoutingProxyContext &);
  void operator=(const HttpRoutingProxyContext &);

  bool _receivedOutboundResponse;
  HttpServerStream *_serverStream;
  HttpClientStream *_clientStream;
  ESB::UInt32 _clientStreamResponseOffset;
  ESB::UInt32 _serverStreamRequestOffset;
};

}  // namespace ES

#endif
