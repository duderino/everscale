/** @file ESFCountingSemaphore.h
 *  @brief An counting semaphore wrapper that also implements the ESFLockable
 *  interface.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_COUNTING_SEMAPHORE_H
#define ESF_COUNTING_SEMAPHORE_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
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

/**    ESFCountingSemaphore realizes the ESFLockable interface with a
 *  counting semaphore.  The maximum count of the semaphore is
 *  4294967295.  Note that this counting semaphore is not asynch
 *  signal safe on unix platforms.
 *
 *  @ingroup lockable
 */
class ESFCountingSemaphore : public ESFLockable {
 public:
  /**    Default constructor. */
  ESFCountingSemaphore();

  /** Default destructor. */
  virtual ~ESFCountingSemaphore();

  /**    Block the calling thread until write access is granted.
   *
   *    @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError writeAcquire();

  /**    Block the calling thread until read access is granted.
   *
   *    @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError readAcquire();

  /**    Attempt to gain write access, returning immediately if access could not
   *    be granted.
   *
   *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual ESFError writeAttempt();

  /**    Attempt to gain read access, returning immediately if access could not
   *    be granted.
   *
   *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual ESFError readAttempt();

  /**    Release the lock after write access was granted.
   *
   *    @return ESF_SUCCESS if successful, another error code otherwise.
   *      ESF_OVERFLOW will be returned if the maximum count is reached.
   */
  virtual ESFError writeRelease();

  /**    Release the lock after read access was granted.
   *
   *    @return ESF_SUCCESS if successful, another error code otherwise.
   *      ESF_OVERFLOW will be returned if the maximum count is reached.
   */
  virtual ESFError readRelease();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  ESFCountingSemaphore(const ESFCountingSemaphore &);
  ESFCountingSemaphore &operator=(const ESFCountingSemaphore &);

#ifdef HAVE_SEM_T
  typedef sem_t CountingSemaphore;
#elif defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
  typedef struct {
    ESFInt32 _count;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
  } CountingSemaphore;
#elif defined HAVE_HANDLE
  typedef HANDLE CountingSemaphore;
#else
#error "sem_t or equivalent is required"
#endif

  CountingSemaphore _semaphore;
  ESFUInt8 _magic;
};

#endif /* ! ESF_COUNTING_SEMAPHORE_H */
