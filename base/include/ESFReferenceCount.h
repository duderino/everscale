/** @file ESFReferenceCount.h
 *  @brief Any class that extends ESFReferenceCount can be used with
 *      ESFSmartPointer.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_REFERENCE_COUNT_H
#define ESF_REFERENCE_COUNT_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_SHARED_COUNTER_H
#include <ESFSharedCounter.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

/** @defgroup smart_ptr Smart Pointers */

/** Any class that extends ESFReferenceCount can be used with ESFSmartPointer
 *
 *    @see ESFSmartPointer
 *  @ingroup smart_ptr
 */
class ESFReferenceCount {
    friend class ESFSmartPointer;

public:

    /** Default constructor.
     */
    ESFReferenceCount();

    /** Destructor.
     */
    virtual ~ESFReferenceCount();

    /** Increment the reference count.
     */
    inline void inc() {
        _refCount.inc();
    }

    /** Decrement the reference count.
     */
    inline void dec() {
        _refCount.dec();
    }

    /** Decrement the reference count and return true if new count is zero.
     *
     *    @return true if count is zero after the decrement.
     */
    inline bool decAndTest() {
        bool result = _refCount.decAndTest();
        return result;
    }

    /** Operator new
     *
     *  @param size The size of the object
     *  @return The new object or NULL if the memory allocation failed.
     */
    inline void *operator new(size_t size) throw() {
        ESFReferenceCount *object = (ESFReferenceCount *) ESFSystemAllocator::GetInstance()->allocate(size);

        if (object)
            object->_allocator = ESFSystemAllocator::GetInstance();

        return object;
    }

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL if the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) throw() {
        ESFReferenceCount *object = (ESFReferenceCount *) allocator->allocate(size);

        if (object)
            object->_allocator = allocator;

        return object;
    }

    /** Operator delete
     *
     *  @param object The object to delete.
     */
    inline void operator delete(void *object) {
        if (((ESFReferenceCount *) object)->_allocator) {
            ((ESFReferenceCount *) object)->_allocator->deallocate(object);
        }
    }

private:

    //  Disabled
    ESFReferenceCount(const ESFReferenceCount &);
    ESFReferenceCount &operator=(const ESFReferenceCount &);

    ESFSharedCounter _refCount;
    ESFAllocator *_allocator;
};

#endif
