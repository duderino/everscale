#ifndef ESB_DELAYED_COMMAND_H
#define ESB_DELAYED_COMMAND_H

#ifndef ESB_COMMAND_H
#include <ESBCommand.h>
#endif

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

namespace ESB {

/** The interface for commands that can be executed after a relative millisecond delay
 */
class DelayedCommand : public Command {
 public:
  /** Constructor
   *
   */
  DelayedCommand();

  /** Destructor.
   */
  virtual ~DelayedCommand();

  inline UInt32 tick() const { return _tick; }

  inline void setTick(UInt32 tick) { _tick = tick; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  DelayedCommand(const DelayedCommand &);
  DelayedCommand &operator=(const DelayedCommand &);

  UInt32 _tick;
};

}  // namespace ESB

#endif
