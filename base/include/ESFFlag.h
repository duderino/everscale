/** @file ESFFlag.h
 *  @brief An atomic flag
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

#ifndef ESF_FLAG_H
#define ESF_FLAG_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

/** An atomic flag.
 *
 *  @ingroup thread
 */
class ESFFlag {
public:

    /** Constructor.
     *
     * @param value The initial value
     */
    ESFFlag(bool value);

    /** Destructor.
     */
    virtual ~ESFFlag();

    inline void set(bool value) {
        _value = value;
    }

    inline bool get() const {
        return _value;
    }

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
    ESFFlag(const ESFFlag &);
    ESFFlag &operator=(const ESFFlag &);

    volatile ESFWord _value;
};

#endif /* ! ESF_FLAG_H */
