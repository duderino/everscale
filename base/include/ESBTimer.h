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

  inline bool inTimingWheel() const { return 0 <= _tick; }

  inline void remove() { _tick = -1; }

  inline Int32 tick() const { return _tick; }

  inline void setTick(Int32 tick) { _tick = tick; }

  inline void *context() const { return _context; }

  inline void setContext(void *context) { _context = context; }

  virtual CleanupHandler *cleanupHandler() { return NULL; }

 private:
  Int32 _tick;
  void *_context;

  ESB_DEFAULT_FUNCS(Timer);
};

}  // namespace ESB

#endif
