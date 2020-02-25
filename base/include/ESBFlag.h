#ifndef ESB_FLAG_H
#define ESB_FLAG_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** An atomic flag.
 *
 *  @ingroup thread
 */
class Flag {
 public:
  /** Constructor.
   *
   * @param value The initial value
   */
  Flag(bool value);

  /** Destructor.
   */
  virtual ~Flag();

  inline void set(bool value) { _value = value; }

  inline bool get() const { return _value; }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  Flag(const Flag &);
  Flag &operator=(const Flag &);

  volatile Word _value;
};

}  // namespace ESB

#endif
