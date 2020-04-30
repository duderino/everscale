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
  enum class State {
    SERVER_REQUEST_WAIT = 0,
    CLIENT_RESPONSE_WAIT = 1,
    STREAMING = 2,
    FINISHED = 3
  };

  HttpRoutingProxyContext();

  virtual ~HttpRoutingProxyContext();

  inline State state() { return _state; }
  inline HttpServerStream *serverStream() { return _serverStream; }
  inline HttpClientStream *clientStream() { return _clientStream; }

  inline void setState(State state) {
#ifndef NDEBUG
    switch (state) {
      case State::CLIENT_RESPONSE_WAIT:
        assert(State::SERVER_REQUEST_WAIT == _state);
        break;
      case State::STREAMING:
        assert(State::CLIENT_RESPONSE_WAIT == _state);
        break;
      default:
        break;
    }
#endif
    _state = state;
  }

  inline void setServerStream(HttpServerStream *serverStream) {
    _serverStream = serverStream;
  }

  inline void setClientStream(HttpClientStream *clientStream) {
    _clientStream = clientStream;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  HttpRoutingProxyContext(const HttpRoutingProxyContext &);
  void operator=(const HttpRoutingProxyContext &);

  State _state;
  HttpServerStream *_serverStream;
  HttpClientStream *_clientStream;
};

}  // namespace ES

#endif
