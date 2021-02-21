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

  inline void setServerStream(HttpServerStream *serverStream) { _serverStream = serverStream; }

  inline void setClientStream(HttpClientStream *clientStream) { _clientStream = clientStream; }

  bool receivedOutboundResponse() const;

  void setReceivedOutboundResponse(bool receivedOutboundResponse);

  inline ESB::UInt32 requestBodyBytesForwarded() const { return _requestBodyBytesForwarded; }

  inline void addRequestBodyBytesForwarded(ESB::UInt32 requestBodyBytesForwarded) {
    _requestBodyBytesForwarded += requestBodyBytesForwarded;
  }

  inline ESB::UInt32 responseBodyBytesForwarded() const { return _responseBodyBytesForwarded; }

  inline void addResponseBodyBytesForwarded(ESB::UInt32 responseBodyBytesSent) {
    _responseBodyBytesForwarded += responseBodyBytesSent;
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

  HttpServerStream *_serverStream;
  HttpClientStream *_clientStream;
  int _flags;
  ESB::UInt32 _requestBodyBytesForwarded;
  ESB::UInt32 _responseBodyBytesForwarded;
};

}  // namespace ES

#endif
