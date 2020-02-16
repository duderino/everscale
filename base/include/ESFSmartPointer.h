/** @file ESFSmartPointer.h
 *  @brief A smart pointer base class
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_SMART_POINTER_H
#define ESF_SMART_POINTER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_REFERENCE_COUNT_H
#include <ESFReferenceCount.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifdef USE_SMART_POINTER_DEBUGGER

#ifndef ESF_SMART_POINTER_DEBUGGER_H
#include <ESFSmartPointerDebugger.h>
#endif

#define PTR_INC()                                      \
  if (_ptr) {                                          \
    _ptr->inc();                                       \
    ESFSmartPointerDebugger::Instance()->insert(_ptr); \
  }

#define PTR_DEC()                                     \
  if (_ptr && _ptr->decAndTest()) {                   \
    ESFSmartPointerDebugger::Instance()->erase(_ptr); \
    destroy();                                        \
  }

#else /* ! defined USE_SMART_POINTER_DEBUGGER */

#define PTR_INC() \
  if (_ptr) _ptr->inc()

#define PTR_DEC() \
  if (_ptr && _ptr->decAndTest()) destroy()

#endif

/** ESFSmartPointers can only be used with classes that inherit from
 *  ESFReferenceCount.  They must only be used with ESFReferenceCount
 *  subclasses that were dynamically allocated.
 *
 *  @ingroup smart_ptr
 */
class ESFSmartPointer {
 public:
  /** Default Constructor.
   */
  inline ESFSmartPointer() : _ptr(0) {}

  /** Conversion Constructor.
   *
   *    @param ptr A pointer to a dynamically allocated ESFReferenceCount
   *      subclass.
   */
  inline ESFSmartPointer(ESFReferenceCount *ptr) : _ptr(ptr) { PTR_INC(); }

  /** Copy constructor.
   *
   *    @param smartPtr another smart pointer.
   */
  inline ESFSmartPointer(const ESFSmartPointer &smartPtr) {
    _ptr = smartPtr._ptr;

    PTR_INC();
  }

  /** Destructor.
   */
  virtual ~ESFSmartPointer() { PTR_DEC(); }

  /** Assignment operator.
   *
   *    @param smartPtr another smart pointer.
   *    @return this object.
   */
  inline ESFSmartPointer &operator=(const ESFSmartPointer &smartPtr) {
    if (_ptr == smartPtr._ptr) return *this;

    PTR_DEC();

    _ptr = smartPtr._ptr;

    PTR_INC();

    return *this;
  }

  /** Assignment operator.
   *
   *    @param ptr A pointer to a dynamically allocated ESFReferenceCount
   *      subclass.
   *    @return this object.
   */
  inline ESFSmartPointer &operator=(ESFReferenceCount *ptr) {
    if (_ptr == ptr) return *this;

    PTR_DEC();

    _ptr = ptr;

    PTR_INC();

    return *this;
  }

  /** Dereference operator.
   *
   *    @return a reference to the wrapped object.
   */
  inline ESFReferenceCount &operator*() {
    ESF_ASSERT(_ptr);

    return *_ptr;
  }

  /** Special dereference operator.
   *
   *    "ptr->method();" means "( ptr.operator->() )->method();" which is
   *    equivalent to "ptr._ref->method();".
   *
   *    @return a pointer to the wrapped object.
   */
  inline ESFReferenceCount *operator->() {
    ESF_ASSERT(_ptr);

    return _ptr;
  }

  /** Special dereference operator.
   *
   *  "ptr->method();" means "( ptr.operator->() )->method();" which is
   *  equivalent to "ptr._ref->method();".
   *
   *  @return a pointer to the wrapped object.
   */
  inline const ESFReferenceCount *operator->() const {
    ESF_ASSERT(_ptr);

    return _ptr;
  }

  /** Checks whether this smart pointer wraps an object.
   *
   *    @return true if the wrapped object is not null, false otherwise.
   */
  inline bool isNull() const { return 0 == _ptr; }

  /** Set the wrapped object to null, deleting it if it this is the last
   *    reference.
   */
  inline void setNull() {
    PTR_DEC();

    _ptr = 0;
  }

  /** Compare two smart pointers based on the address of their wrapped
   *    objects.
   *
   *    @param smartPtr The smart pointer to compare to this object.
   *    @return true if both smart pointers point to the same object, false
   *        otherwise.
   */
  inline bool operator==(const ESFSmartPointer &smartPtr) const {
    return _ptr == smartPtr._ptr;
  }

 protected:
  inline void destroy() {
    /* Calling ESFReferenceCount::delete( void * ) is the equivalent of:
     *
     *  ESFAllocator *allocator = _ptr->_allocator;
     *
     *  _ptr->~ESFReferenceCount();
     *
     *  allocator->deallocate( _ptr );
     */

    delete _ptr;

    _ptr = 0;
  }

  ESFReferenceCount *_ptr;
};

/** In addition to the ESFSmartPointer class which can be used with any
 *  ESFReferenceCount subclass, an additional macro is defined that can
 *  define class-specific smart pointers.  These class specific smart pointers
 *  can be used to call the wrapped class's methods directly without any
 *  downcasting.  The class specific smart pointers can also participate in
 *  their own inheritance hierarchy that parallels the hierarchy of their
 *  wrapped classes.
 *
 *  @param CLASS The pointer type this smart pointer will encapsulate.
 *  @param CLASS_PTR The name of this smart pointer
 *  @param BASE_PTR The name of the smart pointer this smart pointer inherits
 *      from.  Note that CLASS must be a subclass of the same class that
 *      BASE_PTR wraps.
 *
 *  @ingroup object
 */
#define DEFINE_ESF_SMART_POINTER(CLASS, CLASS_PTR, BASE_PTR) \
  class CLASS_PTR : public BASE_PTR {                        \
   public:                                                   \
    inline CLASS_PTR() : BASE_PTR() {}                       \
                                                             \
    inline CLASS_PTR(CLASS *ptr) : BASE_PTR(ptr) {}          \
                                                             \
    inline CLASS_PTR(const CLASS_PTR &smartPtr) {            \
      _ptr = smartPtr._ptr;                                  \
      PTR_INC();                                             \
    }                                                        \
                                                             \
    virtual ~CLASS_PTR() {}                                  \
                                                             \
    inline CLASS_PTR &operator=(const CLASS_PTR &smartPtr) { \
      if (_ptr == smartPtr._ptr) return *this;               \
      PTR_DEC();                                             \
      _ptr = smartPtr._ptr;                                  \
      PTR_INC();                                             \
      return *this;                                          \
    }                                                        \
                                                             \
    inline CLASS_PTR &operator=(CLASS *ptr) {                \
      if (_ptr == ptr) return *this;                         \
      PTR_DEC();                                             \
      _ptr = ptr;                                            \
      PTR_INC();                                             \
      return *this;                                          \
    }                                                        \
                                                             \
    inline CLASS &operator*() {                              \
      ESF_ASSERT(_ptr);                                      \
      return *((CLASS *)_ptr);                               \
    }                                                        \
                                                             \
    inline CLASS *operator->() {                             \
      ESF_ASSERT(_ptr);                                      \
      return (CLASS *)_ptr;                                  \
    }                                                        \
                                                             \
    inline const CLASS *operator->() const {                 \
      ESF_ASSERT(_ptr);                                      \
      return (const CLASS *)_ptr;                            \
    }                                                        \
                                                             \
    inline bool isNull() const { return 0 == _ptr; }         \
                                                             \
    inline void setNull() {                                  \
      PTR_DEC();                                             \
      _ptr = 0;                                              \
    }                                                        \
  }

#endif /* ! ESF_SMART_POINTER_H */
