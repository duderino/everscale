#ifndef ESB_COUNTING_SEMAPHORE_H
#define ESB_COUNTING_SEMAPHORE_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

#ifdef HAVE_SEMAPHORE_H
#include <semaphore.h>
#endif

#if !defined HAVE_SEMAPHORE_H && defined HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace ESB {

/** CountingSemaphore realizes the Lockable interface with a
 *  counting semaphore.  The maximum count of the semaphore is
 *  4294967295.  Note that this counting semaphore is not asynch
 *  signal safe on unix platforms.
 *
 *  @ingroup lockable
 */
class CountingSemaphore : public Lockable {
 public:
  /**    Default constructor. */
  CountingSemaphore();

  /** Default destructor. */
  virtual ~CountingSemaphore();

  /**    Block the calling thread until write access is granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error writeAcquire();

  /**    Block the calling thread until read access is granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error readAcquire();

  /**    Attempt to gain write access, returning immediately if access could not
   *    be granted.
   *
   *  @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error writeAttempt();

  /**    Attempt to gain read access, returning immediately if access could not
   *    be granted.
   *
   *  @return ESB_SUCCESS if access was granted, ESB_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual Error readAttempt();

  /**    Release the lock after write access was granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   *      ESB_OVERFLOW will be returned if the maximum count is reached.
   */
  virtual Error writeRelease();

  /**    Release the lock after read access was granted.
   *
   *    @return ESB_SUCCESS if successful, another error code otherwise.
   *      ESB_OVERFLOW will be returned if the maximum count is reached.
   */
  virtual Error readRelease();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  CountingSemaphore(const CountingSemaphore &);
  CountingSemaphore &operator=(const CountingSemaphore &);

#ifdef HAVE_SEM_T
  typedef sem_t semaphore;
#elif defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
  typedef struct {
    Int32 _count;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
  } semaphore;
#elif defined HAVE_HANDLE
  typedef HANDLE semaphore;
#else
#error "sem_t or equivalent is required"
#endif

  semaphore _semaphore;
  UInt8 _magic;
};

}  // namespace ESB

#endif
