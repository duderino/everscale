#ifndef ES_HTTP_ECHO_CLIENT_CONTEXT_H
#define ES_HTTP_ECHO_CLIENT_CONTEXT_H

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ES {

class HttpEchoClientContext : public ESB::Object {
 public:
  HttpEchoClientContext(ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpEchoClientContext();

  inline unsigned int bytesSent() { return _bytesSent; }

  inline void setBytesSent(unsigned int bytesSent) { _bytesSent = bytesSent; }

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

  /**
   * Atomically decrement the remaining and return the value pre-decrement.
   *
   * @return the value before the decrement.
   */
  static inline int DecrementIterations() { return _RemainingIterations.dec(); }

  static inline bool IsFinished() { return 0 >= _RemainingIterations.get(); }

  static inline void SetIterations(int remainingIterations) {
    _RemainingIterations.set(remainingIterations);
  }

 private:
  // Disabled
  HttpEchoClientContext(const HttpEchoClientContext &context);
  void operator=(const HttpEchoClientContext &context);

  unsigned int _bytesSent;
  ESB::CleanupHandler &_cleanupHandler;
  static ESB::SharedInt _RemainingIterations;
};

}  // namespace ES

#endif
