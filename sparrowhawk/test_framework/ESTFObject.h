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
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:19 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESTF_OBJECT_H
#define ESTF_OBJECT_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

/** Any class in the ESTF test suite can inherit from ESTFObject to
 *  be used with ESTFObjectPtr and it's macros.  Note that this must only
 *  be used within the ESTF test suite as the reference counter is not
 *  threadsafe.
 *
 *  @ingroup test
 */
class ESTFObject
{
public:

    /** Default constructor. */
    inline ESTFObject() : _refCount(0)
    {
    }

    /** Destructor. */
    virtual ~ESTFObject()
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

    ESTFObject( const ESTFObject & );
    ESTFObject &operator=( const ESTFObject & );

    unsigned int _refCount;
};

#endif /* ! ESTF_OBJECT_H */
