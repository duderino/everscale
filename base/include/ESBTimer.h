#ifndef ESB_TIMER_H
#define ESB_TIMER_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ESB {

/** The interface for objects that timeout/activate after a delay
 */
class Timer : public EmbeddedListElement {
 public:
  /** Constructor
   *
   */
  Timer();

  Timer(void *context);

  /** Destructor.
   */
  virtual ~Timer();

  inline UInt32 tick() const { return _tick; }

  inline void setTick(UInt32 tick) { _tick = tick; }

  inline void *context() const { return _context; }

  inline void setContext(void *context) { _context = context; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  UInt32 _tick;
  void *_context;

  ESB_DISABLE_AUTO_COPY(Timer);
};

}  // namespace ESB

#endif
