/** @file ESFMutex.h
 *  @brief A simple mutual exclusion implementation of the ESFLockable interface
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

#ifndef ESF_MUTEX_H
#define ESF_MUTEX_H

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

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

/**    ESFMutex realizes the ESFLockable interface with a simple exclusive
 *  lock mechanism.  Both read acquistions and write acquisitions perform
 *  the same operation.
 *
 *  @ingroup lockable
 */
class ESFMutex : public ESFLockable {
 public:
  /**    Default constructor. */
  ESFMutex();

  /** Default destructor. */
  virtual ~ESFMutex();

  /** Block the calling thread until write access is granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError writeAcquire();

  /** Block the calling thread until read access is granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError readAcquire();

  /** Attempt to gain write access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual ESFError writeAttempt();

  /** Attempt to gain read access, returning immediately if access could not
   *  be granted.
   *
   *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
   *      not be immediately granted, or another error code if an error
   *      occurred.
   */
  virtual ESFError readAttempt();

  /** Release the lock after write access was granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError writeRelease();

  /** Release the lock after read access was granted.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
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
  //  disabled
  ESFMutex(const ESFMutex &);
  ESFMutex &operator=(const ESFMutex &);

#ifdef HAVE_PTHREAD_MUTEX_T
  typedef pthread_mutex_t Mutex;
#elif defined HAVE_HANDLE
  typedef HANDLE Mutex;
#endif

  Mutex _mutex;
  ESFUInt8 _magic;
};

#endif /* ! ESF_MUTEX_H */
