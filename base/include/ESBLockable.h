#ifndef ESB_LOCKABLE_H
#define ESB_LOCKABLE_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

namespace ESB {

/** @defgroup lockable Synchronization
 */

/** Lockable defines the interface that all locks must realize.  Though
 *  it distinguishes between read and write access, concrete realizations of
 *  the interface are free to implement these operations as they see fit (e.g.,
 *  a mutex would do the same thing for both read and write accesses).
 *
 *  No guarantees are made that any realizing subclass operates recursively.
 *  That is, subclasses may act like a recursive mutex and allow the same thread
 *  to acquire the lock many times or they may deadlock the calling thread if
 *  it attempts to acquire the lock more than once.
 *
 *  @ingroup lockable
 */
class Lockable {
 public:
  /** Default constructor.
   */
  Lockable();

  /** Default destructor.
   */
  virtual ~Lockable();

  /**    Block the calling thread until write access is granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error writeAcquire() = 0;

  /**    Block the calling thread until read access is granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error readAcquire() = 0;

  /**    Attempt to gain write access, returning immediately if access could not
   *    be granted.
   *
   *  @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error writeAttempt() = 0;

  /**    Attempt to gain read access, returning immediately if access could not
   *    be granted.
   *
   *    @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error readAttempt() = 0;

  /**    Release the lock after write access was granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error writeRelease() = 0;

  /**    Release the lock after read access was granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error readRelease() = 0;

  ESB_DISABLE_AUTO_COPY(Lockable);
};

}  // namespace ESB

#endif
