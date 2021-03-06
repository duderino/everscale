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

  inline ESB::UInt64 requestBodyBytesForwarded() const { return _requestBodyBytesForwarded; }

  inline void addRequestBodyBytesForwarded(ESB::UInt64 requestBodyBytesForwarded) {
    _requestBodyBytesForwarded += requestBodyBytesForwarded;
  }

  inline ESB::UInt64 responseBodyBytesForwarded() const { return _responseBodyBytesForwarded; }

  inline void addResponseBodyBytesForwarded(ESB::UInt64 responseBodyBytesSent) {
    _responseBodyBytesForwarded += responseBodyBytesSent;
  }

 private:
  HttpServerStream *_serverStream;
  HttpClientStream *_clientStream;
  int _flags;
  ESB::UInt64 _requestBodyBytesForwarded;
  ESB::UInt64 _responseBodyBytesForwarded;

  ESB_DEFAULT_FUNCS(HttpRoutingProxyContext);
};

}  // namespace ES

#endif
