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

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 protected:
  virtual void run();

  /** Should be called from a subclass's run method on every iteration to
   *  determine whether it should return from its run method.
   *
   *  @return true if a stop has been requested, false otherwise.
   */
  bool stopRequested();

 private:
  //  Disabled
  CommandThread(const CommandThread &);
  CommandThread &operator=(const CommandThread &);

  Command *_command;
};

}  // namespace ESB

#endif
