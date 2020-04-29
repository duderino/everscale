#ifndef ES_HTTP_LOADGEN_CONTEXT_H
#define ES_HTTP_LOADGEN_CONTEXT_H

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ES {

class HttpLoadgenContext : public ESB::Object {
 public:
  HttpLoadgenContext(ESB::CleanupHandler &cleanupHandler);

  virtual ~HttpLoadgenContext();

  inline ESB::UInt32 bytesSent() { return _bytesSent; }

  inline void setBytesSent(ESB::UInt32 bytesSent) { _bytesSent = bytesSent; }

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
  static inline int DecRemainingIterations() {
    return _RemainingIterations.dec();
  }

  static inline void IncCompletedIterations() { _CompletedIterations.inc(); }

  static inline bool IsFinished() {
    int completedIterations = _CompletedIterations.get();
    return _TotalIterations <= completedIterations;
  }

  static inline void SetTotalIterations(int totalIterations) {
    _TotalIterations = totalIterations;
    _RemainingIterations.set(totalIterations);
  }

 private:
  // Disabled
  HttpLoadgenContext(const HttpLoadgenContext &context);
  void operator=(const HttpLoadgenContext &context);

  ESB::UInt32 _bytesSent;
  ESB::CleanupHandler &_cleanupHandler;
  static volatile int _TotalIterations;
  static ESB::SharedInt _RemainingIterations;
  static ESB::SharedInt _CompletedIterations;
};

}  // namespace ES

#endif
