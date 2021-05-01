#ifndef ES_HTTP_ORIGIN_CONTEXT_H
#define ES_HTTP_ORIGIN_CONTEXT_H

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ES {

class HttpOriginContext {
 public:
  HttpOriginContext();

  virtual ~HttpOriginContext();

  inline ESB::UInt64 bytesSent() const { return _bytesSent; }

  inline void setBytesSent(ESB::UInt64 bytesSent) { _bytesSent = bytesSent; }

  inline ESB::UInt64 bytesReceived() const { return _bytesReceived; }

  inline void setBytesReceived(ESB::UInt64 bytesReceived) { _bytesReceived = bytesReceived; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  HttpOriginContext(const HttpOriginContext &);
  void operator=(const HttpOriginContext &);

  ESB::UInt64 _bytesSent;
  ESB::UInt64 _bytesReceived;
};

}  // namespace ES

#endif