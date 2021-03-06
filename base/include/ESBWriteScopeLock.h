#ifndef ESB_WRITE_SCOPE_LOCK_H
#define ESB_WRITE_SCOPE_LOCK_H

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

namespace ESB {

/** WriteScopeLock acquires a write lock on a Lockable instance in its
 *  constructor and automatically releases that lock in its destructor.
 *
 *  @ingroup lockable
 */
class WriteScopeLock {
 public:
  /**    Default constructor.
   *
   *    @param lockable The lockable instance to lock/unlock.
   */
  WriteScopeLock(Lockable &lockable) : _lockable(lockable) { _lockable.writeAcquire(); }

  /** Default destructor. */
  virtual ~WriteScopeLock() { _lockable.writeRelease(); }

 private:
  Lockable &_lockable;

  ESB_DISABLE_AUTO_COPY(WriteScopeLock);
};

}  // namespace ESB

#endif
