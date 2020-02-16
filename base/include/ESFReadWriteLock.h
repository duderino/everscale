/** @file ESFReadWriteLock.h
 *  @brief A multiple readers, single writer implementation of the ESFLockable
 *      interface
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

#ifndef ESF_READ_WRITE_LOCK_H
#define ESF_READ_WRITE_LOCK_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

/**    ESFReadWriteLock realizes the ESFLockable interface with a multiple
 *  readers, single writer locking mechanism.  Write locks have preference
 *  over read locks.
 *
 *  @ingroup lockable
 */
class ESFReadWriteLock : public ESFLockable {
 public:
  /**    Default constructor. */
  ESFReadWriteLock();

  /** Default destructor. */
  virtual ~ESFReadWriteLock();

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

 private:
  //  Disabled
  ESFReadWriteLock(const ESFReadWriteLock &);
  ESFReadWriteLock &operator=(const ESFReadWriteLock &);

#ifdef HAVE_PTHREAD_RWLOCK_T
  typedef pthread_rwlock_t ReadWriteLock;
#elif defined HAVE_PTHREAD_MUTEX_T && defined HAVE_PTHREAD_COND_T
  typedef struct {
    ESFUInt16 _readersActive;
    ESFUInt16 _readersWaiting;
    ESFUInt16 _writersActive;
    ESFUInt16 _writersWaiting;
    pthread_mutex_t _mutex;
    pthread_cond_t _readSignal;
    pthread_cond_t _writeSignal;
  } ReadWriteLock;
#else
#error "pthread_rwlock_t or equivalent is required"
#endif

  ReadWriteLock _lock;
  ESFUInt8 _magic;
};

#endif /* ! ESF_MUTEX_H */
