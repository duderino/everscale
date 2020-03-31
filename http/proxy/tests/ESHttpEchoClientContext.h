#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#define ES_HTTP_ECHO_CLIENT_CONTEXT_H

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ES {

class HttpEchoClientContext : public ESB::Object {
 public:
  HttpEchoClientContext(unsigned int remainingIterations,
                        ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpEchoClientContext();

  inline unsigned int getBytesSent() { return _bytesSent; }

  inline void setBytesSent(unsigned int bytesSent) { _bytesSent = bytesSent; }

  inline unsigned int getRemainingIterations() { return _iterations; }

  inline void setRemainingIterations(unsigned int iterations) {
    _iterations = iterations;
  }

  inline ESB::CleanupHandler &cleanupHandler() const { return _cleanupHandler; }

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
  HttpEchoClientContext(const HttpEchoClientContext &context);
  void operator=(const HttpEchoClientContext &context);

  unsigned int _bytesSent;
  unsigned int _iterations;
  ESB::CleanupHandler &_cleanupHandler;
};

}  // namespace ES

#endif
