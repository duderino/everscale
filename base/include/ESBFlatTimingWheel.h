#ifndef ESB_FLAT_TIMING_WHEEL_H
#define ESB_FLAT_TIMING_WHEEL_H

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

#ifndef ESB_DELAYED_COMMAND_H
#include <ESBDelayedCommand.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

namespace ESB {

/** A flat timing wheel for O(1) insertion, bookkeeping and removal at the expense of more memory usage which in turn
 * favors narrower windows / max delays.  See "Hashed and Hierarchical Timing Wheels: Data Structures
for the Efficient Implementation of a Timer Facility" by Varghese et al.
 *
 *  @ingroup util
 */
class FlatTimingWheel {
 public:
  /** Constructor.
   */
  FlatTimingWheel(UInt32 ticks, UInt32 tickMilliSeconds, Allocator &allocator = SystemAllocator::Instance());

  /** Destructor.
   */
  virtual ~FlatTimingWheel();

  /** Insert/schedule a delayed command.  O(1).
   *
   *  @param command The command to execute after a delay.
   *  @param delayMilliSeconds the milliseconds to wait before executing the command
   *  @param maskErrors if true, delays that exceed the timing wheel's window will be reduced to the window max
   *  @return ESB_SUCCESS if successful, ESB_INVALID_ARGUMENT if delay is outside of the timing wheel's range.  If
   * maskErrors is true, ESB_SUCCESS will always be returned.
   */
  Error insert(DelayedCommand *command, UInt32 delayMilliSeconds, bool maskErrors = false);

  /**
   * Remove/cancel a delayed command.  O(1) assuming every command has a unique tick, O(n) for all commands that share
   * the same tick.  Here a 'tick' is a time unit expressed in milliseconds passed to the constructor.
   *
   * @param command to remove.
   * @return
   */
  Error remove(DelayedCommand *command);

  /**
   * Remove all commands from the timing wheel, calling their cleanup handlers in the process.
   */
  void clear();

  /** Excecute all commands that have reached or exceeded their requested delay
   *
   * @param isRunning This object will return true as long as the controlling thread isRunning, false when the
   * controlling thread wants to shutdown.
   *  @return ESB_SUCCESS if successful, another error error code otherwise.
   */
  Error run(SharedInt *isRunning);

  inline UInt32 maxDelayMilliSeconds() { return _tickMilliSeconds * _ticks; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  // Disabled
  FlatTimingWheel(const FlatTimingWheel &);
  FlatTimingWheel &operator=(const FlatTimingWheel &);

  inline UInt32 idx(Int32 value) { return (value % _ticks + _ticks) % _ticks; }

  inline UInt32 ticks(Date date) {
    UInt32 ticks = date.seconds() * 1000 / _tickMilliSeconds;
    ticks += date.microSeconds() / 1000 / _tickMilliSeconds;
    return ticks;
  }

  const UInt32 _tickMilliSeconds;
  const UInt32 _ticks;  // 1 bucket per tick
  UInt32 _currentTick;
  EmbeddedList *_commands;
  Allocator &_allocator;
  Date _currentTickTime;
};

}  // namespace ESB

#endif
