#ifndef ESB_SHARED_COUNTER_H
#define ESB_SHARED_COUNTER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifdef HAVE_ATOMIC_H
#include <atomic>
#elif !defined HAVE_X86_ASM
#include <ESBMutex.h>
#endif

namespace ESB {

/** @defgroup counter Counters */

/** SharedCounter is an integer type that can be safely accessed by
 *  multiple threads.
 *
 *  @ingroup counter
 */
class SharedCounter {
 public:
  /**    Default constructor. */
  SharedCounter();

  SharedCounter(SharedCounter &counter);

  virtual ~SharedCounter();

  SharedCounter &operator=(SharedCounter &counter);

  inline void set(int value) { _counter = value; }

  inline int get() const { return _counter; }

  /**
   * Atomically increment the counter and return the value pre-increment.
   *
   * @return the value before the increment.
   */
  inline int inc() {
    unsigned int counter = 0;

#ifdef HAVE_ATOMIC_T
    counter = ++_counter;
#elif defined HAVE_X86_ASM
    __asm__ __volatile__("lock; incl %0; movl %0, %1"
                         : "=m"(_counter), "=r"(counter)
                         : "m"(_counter)
                         : "memory");
#else
    _lock.writeAcquire();
    counter = ++_counter;
    _lock.writeRelease();
#endif

    return counter;
  }

  /**
   * Atomically decrement the counter and return the value pre-decrement.
   *
   * @return the value before the decrement.
   */
  inline int dec() {
    unsigned int counter = 0;

#ifdef HAVE_ATOMIC_T
    counter = --_counter;
#elif defined HAVE_X86_ASM
    __asm__ __volatile__("lock; decl %0; movl %0, %1"
                         : "=m"(_counter), "=r"(counter)
                         : "m"(_counter)
                         : "memory");
#else
    _lock.writeAcquire();
    counter = --_counter;
    _lock.writeRelease();
#endif

    return counter;
  }

  inline void add(int value) {
#ifdef HAVE_ATOMIC_T
    _counter += value;
#elif defined HAVE_X86_ASM
    __asm__ __volatile__("lock ; addl %1,%0"
                         : "=m"(_counter)
                         : "ir"(value), "m"(_counter));
#else
    _lock.writeAcquire();
    _counter += value;
    _lock.writeRelease();
#endif
  }

  inline void sub(int value) {
#ifdef HAVE_ATOMIC_T
    _counter -= value;
#elif defined HAVE_X86_ASM
    __asm__ __volatile__("lock ; subl %1,%0"
                         : "=m"(_counter)
                         : "ir"(value), "m"(_counter));
#else
    _lock.writeAcquire();
    _counter -= value;
    _lock.writeRelease();
#endif
  }

  inline bool decAndTest() {
    unsigned char c;
#ifdef HAVE_ATOMIC_T
    c = --_counter;
#elif defined HAVE_X86_ASM
    __asm__ __volatile__("lock ; decl %0; sete %1"
                         : "=m"(_counter), "=qm"(c)
                         : "m"(_counter)
                         : "memory");
#else
    _lock.writeAcquire();
    c = --_counter;
    _lock.writeRelease();
#endif
    return c != 0;
  }

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

#ifdef HAVE_ATOMIC_T
 std::atomic<int> _counter;
#else
  #ifndef HAVE_X86_ASM
  Mutex _lock;
  #endif

  volatile int _counter;
#endif
};

}  // namespace ESB

#endif
