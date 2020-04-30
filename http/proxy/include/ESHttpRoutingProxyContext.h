#ifndef ES_HTTP_PROXY_CONTEXT_H
#define ES_HTTP_PROXY_CONTEXT_H

#ifndef ES_HTTP_STREAM_H
#include <ESHttpStream.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ES {

class HttpRoutingProxyContext {
 public:
  HttpRoutingProxyContext();

  virtual ~HttpRoutingProxyContext();

  inline HttpStream *inboundStream() { return _inboundStream; }

  inline void setInboundStream(HttpStream *inboundStream) {
    _inboundStream = inboundStream;
  }

  inline HttpStream *outboundStream() { return _outboundStream; }

  inline void setOutboundStream(HttpStream *outboundStream) {
    _outboundStream = outboundStream;
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

  HttpStream *_inboundStream;
  HttpStream *_outboundStream;
};

}  // namespace ES

#endif
