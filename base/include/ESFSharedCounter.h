/** @file ESFSharedCounter.h
 *  @brief A threadsafe counter
 *
 * Copyright (c) 2011 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SHARED_COUNTER_H
#define ESF_SHARED_COUNTER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef HAVE_X86_ASM

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#endif

/** @defgroup counter Counters */

/** ESFSharedCounter is an integer type that can be safely accessed by
 *  multiple threads.
 *
 *  @ingroup counter
 */
class ESFSharedCounter {
 public:
  /**    Default constructor. */
  ESFSharedCounter();

  ESFSharedCounter(ESFSharedCounter &counter);

  virtual ~ESFSharedCounter();

  ESFSharedCounter &operator=(ESFSharedCounter &counter);

  inline void set(int value) { _counter = value; }

  inline int get() const { return _counter; }

  inline int inc() {
    unsigned int counter = 0;

#ifdef HAVE_X86_ASM
    __asm__ __volatile__("lock; incl %0; movl %0, %1"
                         : "=m"(_counter), "=r"(counter)
                         : "m"(_counter)
                         : "memory");
#else
    _lock.writeAcquire();

    counter = _counter++;

    _lock.writeRelease();
#endif

    return counter;
  }

  inline int dec() {
    unsigned int counter = 0;

#ifdef HAVE_X86_ASM
    __asm__ __volatile__("lock; decl %0; movl %0, %1"
                         : "=m"(_counter), "=r"(counter)
                         : "m"(_counter)
                         : "memory");
#else
    _lock.writeAcquire();

    counter = _counter--;

    _lock.writeRelease();
#endif

    return counter;
  }

  inline void add(int value) {
#ifdef HAVE_X86_ASM
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
#ifdef HAVE_X86_ASM
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

    __asm__ __volatile__("lock ; decl %0; sete %1"
                         : "=m"(_counter), "=qm"(c)
                         : "m"(_counter)
                         : "memory");
    return c != 0;
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
#ifndef HAVE_X86_ASM
  ESFMutex _lock;
#endif

  volatile int _counter;
};

#endif /* ! ESF_SHARED_COUNTER_H */
