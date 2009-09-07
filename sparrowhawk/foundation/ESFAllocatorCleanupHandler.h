/** @file ESFAllocatorCleanupHandler.h
 *  @brief An object that can destroy another object created by an allocator
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

#ifndef ESF_ALLOCATOR_CLEANUP_HANDLER_H
#define ESF_ALLOCATOR_CLEANUP_HANDLER_H

#ifndef ESF_CLEANUP_HANDLER_H
#include <ESFCleanupHandler.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

/** An object that can destroy another object created by an allocator
 *
 *  @ingroup collection
 */
class ESFAllocatorCleanupHandler: public ESFCleanupHandler {
public:

    /** Constructor
     *
     * @param allocator The allocator to free the object with
     */
    ESFAllocatorCleanupHandler(ESFAllocator *allocator);

    /** Destructor
     */
    virtual ~ESFAllocatorCleanupHandler();

    /** Destroy an object
     *
     * @param object The object to destroy
     */
    virtual void destroy(ESFObject *object);

private:
    // Disabled
    ESFAllocatorCleanupHandler(const ESFAllocatorCleanupHandler &);
    void operator=(const ESFAllocatorCleanupHandler &);

    ESFAllocator *_allocator;
};

#endif /* ! ESF_ALLOCATOR_CLEANUP_HANDLER_H */
