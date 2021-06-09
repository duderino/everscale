#ifndef ESB_SIGNAL_HANDLER_H
#define ESB_SIGNAL_HANDLER_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
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

 private:
  /** Default constructor.
   */
  SignalHandler();

  static SignalHandler _Instance;

  ESB_DEFAULT_FUNCS(SignalHandler);
};

}  // namespace ESB

#endif