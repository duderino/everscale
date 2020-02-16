/** @file ESFNullLock.h
 *  @brief An implementation of the ESFLockable interface that provides no
 *      synchronization whatsoever
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

#ifndef ESF_NULL_LOCK_H
#define ESF_NULL_LOCK_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
#endif

/**    ESFNullLock provides an unsynchronized implementation of the ESFLockable
 *  interface.  It is useful in situations where synchronized and unsynchronized
 *  variants of a resource need to be switched between.
 *
 *  @ingroup lockable
 */
class ESFNullLock : public ESFLockable {
 public:
  /** Get an instance of the ESFNullLock without creating a new object.
   *
   *  @return a ESFNullLock object.
   */
  static ESFNullLock *Instance();

  /**    Default constructor. */
  ESFNullLock();

  /** Default destructor. */
  virtual ~ESFNullLock();

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
  // Disabled
  ESFNullLock(const ESFNullLock &);
  ESFNullLock &operator=(const ESFNullLock &);

  static ESFNullLock _Instance;
};

#endif /* ! ESF_NULL_LOCK_H */
