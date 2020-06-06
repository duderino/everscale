#ifndef ESB_READ_SCOPE_LOCK_H
#define ESB_READ_SCOPE_LOCK_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

namespace ESB {

/** ReadScopeLock acquires a read lock on a ESBLockable instance in its
 *  constructor and automatically releases that lock in its destructor.
 *
 *  @ingroup lockable
 */
class ReadScopeLock {
 public:
  /**    Default constructor.
   *
   *    @param lockable The lockable instance to lock/unlock.
   */
  ReadScopeLock(Lockable &lockable) : _lockable(lockable) { _lockable.readAcquire(); }

  /** Default destructor. */
  virtual ~ReadScopeLock() { _lockable.readRelease(); }

 private:
  //  Disabled
  ReadScopeLock(const ReadScopeLock &);
  ReadScopeLock &operator=(const ReadScopeLock &);

  Lockable &_lockable;
};

}  // namespace ESB

#endif
