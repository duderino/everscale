#ifndef ESB_NULL_LOCK_H
#define ESB_NULL_LOCK_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

namespace ESB {

/** NullLock provides an unsynchronized implementation of the Lockable
 *  interface.  It is useful in situations where synchronized and unsynchronized
 *  variants of a resource need to be switched between.
 *
 *  @ingroup lockable
 */
class NullLock : public Lockable {
 public:
  /** Get an instance of the NullLock without creating a new object.
   *
   *  @return a NullLock object.
   */
  static NullLock &Instance();

  /**    Default constructor. */
  NullLock();

  /** Default destructor. */
  virtual ~NullLock();

  /** Block the calling thread until write access is granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error writeAcquire();

  /** Block the calling thread until read access is granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error readAcquire();

  /** Attempt to gain write access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error writeAttempt();

  /** Attempt to gain read access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error readAttempt();

  /** Release the lock after write access was granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error writeRelease();

  /** Release the lock after read access was granted.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error readRelease();

 private:
  // Disabled
  NullLock(const NullLock &);
  NullLock &operator=(const NullLock &);

  static NullLock _Instance;
};

}  // namespace ESB

#endif
