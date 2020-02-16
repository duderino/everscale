/** @file ESTFObject.h
 *  @brief A reference-counted base class that lets subclasses be used with
 *      smart pointers.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 */

#ifndef ESTF_OBJECT_H
#define ESTF_OBJECT_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

namespace  {

/** Any class in the  test suite can inherit from Object to
 *  be used with ObjectPtr and it's macros.  Note that this must only
 *  be used within the  test suite as the reference counter is not
 *  threadsafe.
 *
 *  @ingroup test
 */
class Object
{
public:

    /** Default constructor. */
    inline Object() : _refCount(0)
    {
    }

    /** Destructor. */
    virtual ~Object()
    {
        ESTF_NATIVE_ASSERT( 0 == _refCount );
    }

    /** Increment the reference count. */
    inline void inc()
    {
        ++_refCount;
    }

    /** Decrement the reference count. */
    inline void dec()
    {
        --_refCount;
    }

    /** Decrement the reference count and return true if new count is zero.
     *
     *    @return true if count is zero after the decrement.
     */
    inline bool decAndTest()
    {
        bool result = ( 0 == --_refCount );

        return result;
    }

    /** Get the current reference count.
     *
     *    @return The current reference count.
     */
    inline int getRefCount()
    {
        return _refCount;
    }

private:

    Object( const Object & );
    Object &operator=( const Object & );

    unsigned int _refCount;
};

}

#endif /* ! _OBJECT_H */
