/** @file ESFLockable.h
 *  @brief A generic lock interface that all synchronization mechanisms must
 *      implement.
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

#ifndef ESF_LOCKABLE_H
#define ESF_LOCKABLE_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

/** @defgroup lockable Synchronization
 */

/**    ESFLockable defines the interface that all locks must realize.  Though
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
class ESFLockable {
public:

    /** Default constructor.
     */
    ESFLockable();

    /** Default destructor.
     */
    virtual ~ESFLockable();

    /**    Block the calling thread until write access is granted.
     *
     *    @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError writeAcquire() = 0;

    /**    Block the calling thread until read access is granted.
     *
     *    @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError readAcquire() = 0;

    /**    Attempt to gain write access, returning immediately if access could not
     *    be granted.
     *
     *  @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
     *      not be immediately granted, or another error code if an error
     *      occurred.
     */
    virtual ESFError writeAttempt() = 0;

    /**    Attempt to gain read access, returning immediately if access could not
     *    be granted.
     *
     *    @return ESF_SUCCESS if access was granted, ESF_AGAIN if access could
     *      not be immediately granted, or another error code if an error
     *      occurred.
     */
    virtual ESFError readAttempt() = 0;

    /**    Release the lock after write access was granted.
     *
     *    @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError writeRelease() = 0;

    /**    Release the lock after read access was granted.
     *
     *    @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError readRelease() = 0;
};

#endif /* ! ESF_LOCKABLE_H */
