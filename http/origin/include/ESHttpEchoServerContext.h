#ifndef ES_HTTP_ECHO_SERVER_CONTEXT_H
#define ES_HTTP_ECHO_SERVER_CONTEXT_H

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ES {

class HttpEchoServerContext {
 public:
  HttpEchoServerContext();

  virtual ~HttpEchoServerContext();

  inline unsigned int getBytesSent() { return _bytesSent; }

  inline void addBytesSent(unsigned int bytesSent) { _bytesSent += bytesSent; }

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
  HttpEchoServerContext(const HttpEchoServerContext &state);
  void operator=(const HttpEchoServerContext &state);

  unsigned int _bytesSent;
};

}  // namespace ES

#endif
