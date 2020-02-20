#ifndef ESTF_OBJECT_H
#define ESTF_OBJECT_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

namespace ESTF {

/** Any class in the unit test suite can inherit from Object to
 *  be used with ObjectPtr and it's macros.  Note that this must only
 *  be used within the  test suite as the reference counter is not
 *  threadsafe.
 *
 *  @ingroup unit-test
 */
class Object {
 public:
  /** Default constructor. */
  inline Object() : _refCount(0) {}

  /** Destructor. */
  virtual ~Object() { assert(0 == _refCount); }

  /** Increment the reference count. */
  inline void inc() { ++_refCount; }

  /** Decrement the reference count. */
  inline void dec() { --_refCount; }

  /** Decrement the reference count and return true if new count is zero.
   *
   *    @return true if count is zero after the decrement.
   */
  inline bool decAndTest() {
    bool result = (0 == --_refCount);

    return result;
  }

  /** Get the current reference count.
   *
   *    @return The current reference count.
   */
  inline int getRefCount() { return _refCount; }

 private:
  Object(const Object &);
  Object &operator=(const Object &);

  unsigned int _refCount;
};

}  // namespace ESTF

#endif /* ! _OBJECT_H */
