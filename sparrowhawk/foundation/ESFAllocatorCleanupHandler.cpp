/** @file ESFAllocatorCleanupHandler.cpp
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
#include <ESFAllocatorCleanupHandler.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

ESFAllocatorCleanupHandler::ESFAllocatorCleanupHandler(ESFAllocator *allocator) :
    _allocator(allocator) {
    ESF_ASSERT(_allocator);
}

ESFAllocatorCleanupHandler::~ESFAllocatorCleanupHandler() {
}

void ESFAllocatorCleanupHandler::destroy(ESFObject *object) {
    if (!_allocator || !object) {
        return;
    }

    object->~ESFObject();
    _allocator->deallocate(object);
}

