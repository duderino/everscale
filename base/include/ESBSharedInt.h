#ifndef ESB_SHARED_INT_H
#define ESB_SHARED_INT_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#if !defined HAVE_X86_ASM && !defined HAVE_GCC_ATOMIC_INTRINSICS
#ifdef HAVE_ATOMIC_H
#include <atomic>
#else
#include <ESBMutex.h>
#endif
#endif

#if !defined HAVE_GCC_ATOMIC_INTRINSICS && defined HAVE_X86_ASM
#define ESB_SHARED_INT_ATTRIBUTE __attribute__((no_sanitize("thread")))
#else
#define ESB_SHARED_INT_ATTRIBUTE
#endif

namespace ESB {

/** @defgroup counter Counters */

/** SharedInt is an integer that can be safely used by multiple threads.
 *
 *  @ingroup counter
 */
class SharedInt {
 public:
  /**    Default constructor. */
  SharedInt();

  SharedInt(UInt32 value);

  SharedInt(SharedInt &counter);

  virtual ~SharedInt();

  SharedInt &operator=(SharedInt &counter);

  inline void set(UInt32 value) ESB_SHARED_INT_ATTRIBUTE {
#if defined HAVE_GCC_ATOMIC_INTRINSICS
    __atomic_store(&_counter, &value, __ATOMIC_RELAXED);
#elif defined HAVE_X86_ASM
    asm volatile("movl %1, %0" : "=r"(_counter) : "r"(value) : "memory");
#elif defined HAVE_ATOMIC_T
    _counter = value;
#else
    _lock.writeAcquire();
    counter = _counter;
    _lock.writeRelease();
#endif
  }

  inline UInt32 get() const ESB_SHARED_INT_ATTRIBUTE {
    UInt32 counter = 0;
#if defined HAVE_GCC_ATOMIC_INTRINSICS
    __atomic_load(&_counter, &counter, __ATOMIC_RELAXED);
#elif defined HAVE_X86_ASM
    asm volatile("movl %1, %0" : "=r"(counter) : "r"(_counter) : "memory");
#elif defined HAVE_ATOMIC_T
    counter = _counter;
#else
    _lock.readAcquire();
    _counter = counter;
    _lock.readRelease();
#endif
    return counter;
  }

  /**
   * Atomically increment the counter and return the value post-increment.
   *
   * @return the value after the increment.
   */
  inline UInt32 inc() ESB_SHARED_INT_ATTRIBUTE {
    UInt32 counter = 0;
#if defined HAVE_GCC_ATOMIC_INTRINSICS
    counter = __atomic_add_fetch(&_counter, 1, __ATOMIC_RELAXED);
#elif defined HAVE_X86_ASM
    asm volatile("lock; incl %0; movl %0, %1" : "=m"(_counter), "=r"(counter) : "m"(_counter) : "memory");
#elif defined HAVE_ATOMIC_T
    counter = ++_counter;
#else
    _lock.writeAcquire();
    counter = ++_counter;
    _lock.writeRelease();
#endif
    return counter;
  }

  /**
   * Atomically decrement the counter and return the value post-decrement.
   *
   * @return the value after the decrement.
   */
  inline UInt32 dec() ESB_SHARED_INT_ATTRIBUTE {
    UInt32 counter = 0;
#if defined HAVE_GCC_ATOMIC_INTRINSICS
    counter = __atomic_sub_fetch(&_counter, 1, __ATOMIC_RELAXED);
#elif defined HAVE_X86_ASM
    asm volatile("lock; decl %0; movl %0, %1" : "=m"(_counter), "=r"(counter) : "m"(_counter) : "memory");
#elif defined HAVE_ATOMIC_T
    counter = --_counter;
#else
    _lock.writeAcquire();
    counter = --_counter;
    _lock.writeRelease();
#endif
    return counter;
  }

  inline void add(UInt32 value) ESB_SHARED_INT_ATTRIBUTE {
#if defined HAVE_GCC_ATOMIC_INTRINSICS
    __atomic_add_fetch(&_counter, value, __ATOMIC_RELAXED);
#elif defined HAVE_X86_ASM
    asm volatile("lock; addl %1,%0" : "=m"(_counter) : "ir"(value), "m"(_counter));
#elif defined HAVE_ATOMIC_T
    _counter += value;
#else
    _lock.writeAcquire();
    _counter += value;
    _lock.writeRelease();
#endif
  }

  inline void sub(UInt32 value) ESB_SHARED_INT_ATTRIBUTE {
#if defined HAVE_GCC_ATOMIC_INTRINSICS
    __atomic_sub_fetch(&_counter, value, __ATOMIC_RELAXED);
#elif defined HAVE_X86_ASM
    asm volatile("lock; subl %1,%0" : "=m"(_counter) : "ir"(value), "m"(_counter));
#elif defined HAVE_ATOMIC_T
    _counter -= value;
#else
    _lock.writeAcquire();
    _counter -= value;
    _lock.writeRelease();
#endif
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
#if defined HAVE_GCC_ATOMIC_INTRINSICS
  volatile UInt32 _counter;
#elif defined HAVE_X86_ASM
  volatile UInt32 _counter;
#elif defined HAVE_ATOMIC_T
  std::atomic<UInt32> _counter;
#else
  Mutex _lock;
  volatile UInt32 _counter;
#endif
};

}  // namespace ESB

#endif
