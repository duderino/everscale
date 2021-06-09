#ifndef ESB_COMMAND_THREAD_H
#define ESB_COMMAND_THREAD_H

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_COMMAND_H
#include <ESBCommand.h>
#endif

namespace ESB {

/** A thread that runs an Command
 *
 *  @ingroup thread
 */
class CommandThread : public Thread {
 public:
  /** Constructor
   *
   * @param command The command to run in a separate thread of control
   */
  CommandThread(Command *command);

  /** Default destructor.
   */
  virtual ~CommandThread();

 protected:
  virtual void run();

  /** Should be called from a subclass's run method on every iteration to
   *  determine whether it should return from its run method.
   *
   *  @return true if a stop has been requested, false otherwise.
   */
  bool stopRequested();

 private:
  Command *_command;

  ESB_DEFAULT_FUNCS(CommandThread);
};

}  // namespace ESB

#endif
