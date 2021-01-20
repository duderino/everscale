#ifndef ESB_SIGNAL_HANDLER_H
#define ESB_SIGNAL_HANDLER_H

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** Default signal handler which exits on sigterm and tries to log a stack trace on fatal signals.
 *
 *  @ingroup thread
 */
class SignalHandler {
 public:
  static SignalHandler &Instance() { return _Instance; }

  /** Default destructor.
   */
  virtual ~SignalHandler();

  /**
   * Install signal handlers
   *
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error initialize();

  /** Return false if the program should shutdown, true otherwise.
   *
   * @return false if the program should shutdown, true otherwise
   */
  bool running();

  /**
   * Request the program shutdown.   This is cooperative, only code that periodically checks the return value of
   * running() will shutdown.
   */
  void stop();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  //  Disabled
  SignalHandler(const SignalHandler &);
  SignalHandler &operator=(const SignalHandler &);

  /** Default constructor.
   */
  SignalHandler();

  static SignalHandler _Instance;
};

}  // namespace ESB

#endif