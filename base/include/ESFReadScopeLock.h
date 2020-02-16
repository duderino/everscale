/** @file ESFReadScopeLock.h
 *  @brief A scope lock that will acquire a ESFLockable's read lock
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_READ_SCOPE_LOCK_H
#define ESF_READ_SCOPE_LOCK_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_LOCKABLE_H
#include <ESFLockable.h>
#endif

/**    ESFReadScopeLock acquires a read lock on a ESFLockable instance in its
 *  constructor and automatically releases that lock in its destructor.
 *
 *  @ingroup lockable
 */
class ESFReadScopeLock {
public:
    /**    Default constructor.
     *
     *    @param lockable The lockable instance to lock/unlock.
     */
    ESFReadScopeLock(ESFLockable &lockable) :
        _lockable(lockable) {
        _lockable.readAcquire();
    }

    /** Default destructor. */
    virtual ~ESFReadScopeLock() {
        _lockable.readRelease();
    }

private:

    //  Disabled
    ESFReadScopeLock(const ESFReadScopeLock &);
    ESFReadScopeLock &operator=(const ESFReadScopeLock &);

    ESFLockable &_lockable;
};

#endif /* ! ESF_READ_SCOPE_LOCK_H */
