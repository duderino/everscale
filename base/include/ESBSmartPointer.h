#ifndef ESB_SMART_POINTER_H
#define ESB_SMART_POINTER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_REFERENCE_COUNT_H
#include <ESBReferenceCount.h>
#endif

#ifdef USE_SMART_POINTER_DEBUGGER

#ifndef ESB_SMART_POINTER_DEBUGGER_H
#include <ESBSmartPointerDebugger.h>
#endif

#define PTR_INC()                                   \
  if (_ptr) {                                       \
    _ptr->inc();                                    \
    SmartPointerDebugger::Instance()->insert(_ptr); \
  }

#define PTR_DEC()                                   \
  if (_ptr && _ptr->decAndTest()) {                 \
    SmartPointerDebugger::Instance()->remove(_ptr); \
    destroy();                                      \
  }

#else /* ! defined USE_SMART_POINTER_DEBUGGER */

#define PTR_INC() \
  if (_ptr) _ptr->inc()

#define PTR_DEC() \
  if (_ptr && _ptr->decAndTest()) destroy()

#endif

namespace ESB {

/** SmartPointers can only be used with classes that inherit from
 *  ReferenceCount.  They must only be used with ReferenceCount
 *  subclasses that were dynamically allocated.
 *
 *  @ingroup smart_ptr
 */
class SmartPointer {
 public:
  /** Default Constructor.
   */
  inline SmartPointer() : _ptr(0) {}

  /** Conversion Constructor.
   *
   *    @param ptr A pointer to a dynamically allocated ReferenceCount
   *      subclass.
   */
  inline SmartPointer(ReferenceCount *ptr) : _ptr(ptr) { PTR_INC(); }

  /** Copy constructor.
   *
   *    @param smartPtr another smart pointer.
   */
  inline SmartPointer(const SmartPointer &smartPtr) {
    _ptr = smartPtr._ptr;
    PTR_INC();
  }

  /** Destructor.
   */
  virtual ~SmartPointer() { PTR_DEC(); }

  /** Assignment operator.
   *
   *    @param smartPtr another smart pointer.
   *    @return this object.
   */
  inline SmartPointer &operator=(const SmartPointer &smartPtr) {
    if (_ptr == smartPtr._ptr) return *this;
    PTR_DEC();
    _ptr = smartPtr._ptr;
    PTR_INC();
    return *this;
  }

  /** Assignment operator.
   *
   *    @param ptr A pointer to a dynamically allocated ReferenceCount
   *      subclass.
   *    @return this object.
   */
  inline SmartPointer &operator=(ReferenceCount *ptr) {
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
  inline ReferenceCount &operator*() {
    assert(_ptr);
    return *_ptr;
  }

  /** Special dereference operator.
   *
   *    "ptr->method();" means "( ptr.operator->() )->method();" which is
   *    equivalent to "ptr._ref->method();".
   *
   *    @return a pointer to the wrapped object.
   */
  inline ReferenceCount *operator->() {
    assert(_ptr);
    return _ptr;
  }

  /** Special dereference operator.
   *
   *  "ptr->method();" means "( ptr.operator->() )->method();" which is
   *  equivalent to "ptr._ref->method();".
   *
   *  @return a pointer to the wrapped object.
   */
  inline const ReferenceCount *operator->() const {
    assert(_ptr);
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
  inline bool operator==(const SmartPointer &smartPtr) const {
    return _ptr == smartPtr._ptr;
  }

 protected:
  inline void destroy() {
    /* Calling ReferenceCount::delete( void * ) is the equivalent of:
     *  Allocator *allocator = _ptr->_allocator;
     *  _ptr->~ReferenceCount();
     *  allocator->deallocate( _ptr );
     */
    delete _ptr;
    _ptr = 0;
  }

  ReferenceCount *_ptr;
};

/** In addition to the SmartPointer class which can be used with any
 *  ReferenceCount subclass, an additional macro is defined that can
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
#define ESB_SMART_POINTER(CLASS, CLASS_PTR, BASE_PTR)        \
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
      assert(_ptr);                                          \
      return *((CLASS *)_ptr);                               \
    }                                                        \
                                                             \
    inline CLASS *operator->() {                             \
      assert(_ptr);                                          \
      return (CLASS *)_ptr;                                  \
    }                                                        \
                                                             \
    inline const CLASS *operator->() const {                 \
      assert(_ptr);                                          \
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
}  // namespace ESB

#endif
