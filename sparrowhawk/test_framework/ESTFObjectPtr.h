/** @file ESTFObjectPtr.h
 *  @brief A smart pointer hierarchy that can be used with classes that
 *      inherit from ESTFObject
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

#ifndef ESTF_OBJECT_PTR_H
#define ESTF_OBJECT_PTR_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifndef ESTF_OBJECT_H
#include <ESTFObject.h>
#endif

/** ESTFObjectPtrs can only be used with classes that inherit from
 *  ESTFObject.  They must only be used with ESTFObject
 *  subclasses that were dynamically allocated.
 *
 *  @ingroup test
 */
class ESTFObjectPtr
{
public:
    /** Default Constructor. */
    inline ESTFObjectPtr() : _ptr(0)
    {
    }

    /** Conversion Constructor.
     *
     *    @param ptr A pointer to a dynamically allocated ESTFObject subclass.
     */
    inline ESTFObjectPtr( ESTFObject *ptr ) : _ptr(ptr)
    {
        if ( _ptr ) _ptr->inc();
    }

    /** Copy constructor.
      *
     *    @param smartPtr another smart pointer.
     */
    inline ESTFObjectPtr( const ESTFObjectPtr &smartPtr )
    {
        _ptr = smartPtr._ptr;

        if ( _ptr ) _ptr->inc();
    }

    /** Destructor. */
    virtual ~ESTFObjectPtr()
    {
        if ( _ptr && _ptr->decAndTest() ) delete _ptr;
    }

    /** Assignment operator.
     *
      *    @param smartPtr another smart pointer.
     *    @return this object.
     */
    inline ESTFObjectPtr &operator=( const ESTFObjectPtr &smartPtr )
    {
        if ( this == &smartPtr ) return *this;

        if ( _ptr && _ptr->decAndTest() ) delete _ptr;

        _ptr = smartPtr._ptr;

        if ( _ptr ) _ptr->inc();

        return *this;
    }

    /** Assignment operator.
     *
     *    @param ptr A pointer to a dynamically allocated ESTFObject subclass.
     *    @return this object.
     */
    inline ESTFObjectPtr &operator=( ESTFObject *ptr )
    {
        if ( _ptr && _ptr->decAndTest() ) delete _ptr;

        if ( ptr ) ptr->inc();

        _ptr = ptr;

        return *this;
    }

    /** Dereference operator.
     *
     *    @return a reference to the wrapped object.
     */
    inline ESTFObject &operator*()
    {
        ESTF_NATIVE_ASSERT( _ptr );

        return *_ptr;
    }

    /** Special dereference operator.
     *
     *    "ptr->method();" means "( ptr.operator->() )->method();" which is
     *    equivalent to "ptr._ref->method();".
     *
     *    @return a pointer to the wrapped object.
     */
    inline ESTFObject *operator->()
    {
        ESTF_NATIVE_ASSERT( _ptr );

        return _ptr;
    }

    /** Special dereference operator.
     *
     *  "ptr->method();" means "( ptr.operator->() )->method();" which is
     *  equivalent to "ptr._ref->method();".
     *
     *  @return a pointer to the wrapped object.
     */
    inline const ESTFObject *operator->() const
    {
        ESTF_NATIVE_ASSERT( _ptr );

        return _ptr;
    }

    /** Checks whether this smart pointer wraps an object.
     *
     *    @return true if the wrapped object is not null, false otherwise.
     */
    inline bool isNull() const
    {
        return 0 == _ptr;
    }

    /** Set the wrapped object to null, deleting it if it this is the last
     *    reference.
     */
    inline void setNull()
    {
        if ( _ptr && _ptr->decAndTest() ) delete _ptr;

        _ptr = 0;
    }

    /** Compare two smart pointers based on the address of their wrapped
     *    objects.
     *
     *    @param smartPtr The smart pointer to compare to this object.
     *    @return true if both smart pointers point to the same object, false
     *        otherwise.
     */
    inline bool operator==( const ESTFObjectPtr &smartPtr ) const
    {
        return _ptr == smartPtr._ptr;
    }

protected:

    ESTFObject *_ptr;
};

/** While the ESTFObjectPtr smart pointer can be used with any ESTFObject
 *  subclass instance, additional class-specific smart pointers can be defined
 *  with ths macro.  These class-specific smart pointers call the wrapped
 *  class's methods without any downcasting.  These class-specific smart
 *  pointers also participate in their own inheritance hierarchy that
 *  parallels the hierarchy of the classes they wrap.
 *
 *  @param CLASS The name of the parallel class for this smart pointer
 *  @param BASE The name of the parallel class's base class (single
 *      inheritance only)
 *  @ingroup test
 */
#define DEFINE_ESTF_OBJECT_PTR(CLASS,BASE) \
class CLASS##Ptr : public BASE##Ptr \
{ \
public: \
    inline CLASS##Ptr() : BASE##Ptr() {} \
    \
    inline CLASS##Ptr( CLASS *ptr ) : BASE##Ptr(ptr) {} \
    \
    inline CLASS##Ptr( const CLASS##Ptr &smartPtr ) \
    {\
        _ptr = smartPtr._ptr;\
        if ( _ptr ) _ptr->inc();\
    }\
    \
    virtual ~CLASS##Ptr() {}\
    \
    inline CLASS##Ptr &operator=( const CLASS##Ptr &smartPtr )\
    {\
        if ( this == &smartPtr ) return *this;\
        if ( _ptr && _ptr->decAndTest() ) delete _ptr;\
        _ptr = smartPtr._ptr;\
        if ( _ptr ) _ptr->inc();\
        return *this;\
    }\
    \
    inline CLASS##Ptr &operator=( CLASS *ptr )\
    {\
        if ( _ptr && _ptr->decAndTest() ) delete _ptr;\
        if ( ptr ) ptr->inc();\
        _ptr = ptr;\
        return *this;\
    }\
    \
    inline CLASS &operator*()\
    {\
        ESTF_NATIVE_ASSERT( _ptr );\
        return *((CLASS *) _ptr);\
    }\
    \
    inline CLASS *operator->()\
    {\
        ESTF_NATIVE_ASSERT( _ptr );\
        return (CLASS *) _ptr;\
    }\
    \
    inline const CLASS *operator->() const\
    {\
        ESTF_NATIVE_ASSERT( _ptr );\
        return (const CLASS *) _ptr;\
    }\
    \
    inline bool isNull() const\
    {\
        return 0 == _ptr;\
    }\
    \
    inline void setNull()\
    {\
        if ( _ptr && _ptr->decAndTest() ) delete _ptr;\
        _ptr = 0;\
    }\
};\

#endif                                 /* ! ESTF_OBJECT_PTR_H */
