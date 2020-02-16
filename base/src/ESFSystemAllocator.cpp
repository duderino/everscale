/** @file ESFSystemAllocator.cpp
 *  @brief An implementation of the ESFAllocator interface that uses the
 *      system's heap
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).vvv
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include "ESFSystemAllocator.h"
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

ESFSystemAllocator ESFSystemAllocator::_Allocator;

ESFSystemAllocator *
ESFSystemAllocator::GetInstance() {
    return &_Allocator;
}

ESFSystemAllocator::ESFSystemAllocator() {
}

ESFSystemAllocator::~ESFSystemAllocator() {
}

void *
ESFSystemAllocator::allocate(ESFUWord size) {
    if (0 == size) {
        return 0;
    }

#ifdef HAVE_MALLOC
    return malloc(size);
#else
#error "Platform requires malloc or equivalent"
#endif
}

ESFError ESFSystemAllocator::allocate(void **block, ESFUWord size) {
    if (!block) {
        return ESF_NULL_POINTER;
    }

    if (0 == size) {
        return ESF_INVALID_ARGUMENT;
    }

    void *tmp = 0;

#ifdef HAVE_MALLOC
    tmp = malloc(size);
#else
#error "Platform requires malloc or equivalent"
#endif

    if (!tmp) {
        return ESF_OUT_OF_MEMORY;
    }

    *block = tmp;

    return ESF_SUCCESS;
}

ESFError ESFSystemAllocator::deallocate(void *block) {
    if (!block) {
        return ESF_NULL_POINTER;
    }

#ifdef HAVE_FREE
    free(block);
#else
#error "Platform requires free or equivalent"
#endif

    return ESF_SUCCESS;
}

ESFUWord ESFSystemAllocator::getOverhead() {
    return ESF_UWORD_C( 0 );
}

ESFError ESFSystemAllocator::initialize() {
    return ESF_SUCCESS;
}

ESFError ESFSystemAllocator::destroy() {
    return ESF_OPERATION_NOT_SUPPORTED;
}

ESFError ESFSystemAllocator::isInitialized() {
    return ESF_SUCCESS;
}

ESFError ESFSystemAllocator::setFailoverAllocator(ESFAllocator *allocator) {
    return ESF_OPERATION_NOT_SUPPORTED;
}

ESFError ESFSystemAllocator::getFailoverAllocator(ESFAllocator **allocator) {
    return ESF_OPERATION_NOT_SUPPORTED;
}
