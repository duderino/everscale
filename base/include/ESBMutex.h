#ifndef ESB_MUTEX_H
#define ESB_MUTEX_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace ESB {

/** Mutex realizes the Lockable interface with a simple exclusive
 *  lock mechanism.  Both read acquistions and write acquisitions perform
 *  the same operation.
 *
 *  @ingroup lockable
 */
class Mutex : public Lockable {
 public:
  /**    Default constructor. */
  Mutex();

  /** Default destructor. */
  virtual ~Mutex();

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
#ifdef HAVE_PTHREAD_MUTEX_T
  typedef pthread_mutex_t mutex;
#elif defined HAVE_HANDLE
  typedef HANDLE mutex;
#endif

  mutex _mutex;
  UInt8 _magic;

  ESB_DEFAULT_FUNCS(Mutex);
};

}  // namespace ESB

#endif
