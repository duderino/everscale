#ifndef ESB_READ_WRITE_LOCK_H
#define ESB_READ_WRITE_LOCK_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** ReadWriteLock realizes the Lockable interface with a multiple
 *  readers, single writer locking mechanism.  Write locks have preference
 *  over read locks.
 *
 *  @ingroup lockable
 */
class ReadWriteLock : public Lockable {
 public:
  /**    Default constructor. */
  ReadWriteLock();

  /** Default destructor. */
  virtual ~ReadWriteLock();

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

  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  //  Disabled
  ReadWriteLock(const ReadWriteLock &);
  ReadWriteLock &operator=(const ReadWriteLock &);

#ifdef HAVE_PTHREAD_RWLOCK_T
  typedef pthread_rwlock_t lock;
#elif defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
  typedef struct {
    UInt16 _readersActive;
    UInt16 _readersWaiting;
    UInt16 _writersActive;
    UInt16 _writersWaiting;
    pthread_mutex_t _mutex;
    pthread_cond_t _readSignal;
    pthread_cond_t _writeSignal;
  } lock;
#else
#error "pthread_rwlock_t or equivalent is required"
#endif

  lock _lock;
  UInt8 _magic;
};

}  // namespace ESB

#endif
