/** @file ESFFixedAllocator.h
 *  @brief A ESFAllocator implementation good for fixed-length allocations.
 *
 * Copyright (c) 2009 Yahoo! Inc.
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

#ifndef ESF_FIXED_ALLOCATOR_H
#define ESF_FIXED_ALLOCATOR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

/** ESFFixedAllocator realizes the ESFAllocator interface with a simple fixed
 *  length allocator.  The ESFFixedAllocator handles memory reservation and
 *  compaction very efficiently with the low overhead of 1 word per
 *  allocation because of its limitation that all allocations are always the
 *  same, fixed size.
 *
 *  @ingroup allocator
 */
class ESFFixedAllocator : public ESFAllocator {
 public:
  /** Constructor.  Creates the ESFFixedAllocator given the number of blocks
   *  and size of each block for its memory pool and also a source allocator
   *  from which it will request its memory pool.  The amount of memory it
   *  will actually request from the source allocator is blocks * blockSize
   *  plus one word per block.
   *
   *  @param blocks The number of blocks this allocator can allocate before
   *      exhausting its memory pool.
   *  @param blockSize The size in bytes of each block.
   *  @param source The allocator to use to allocate the memory pool.
   */
  ESFFixedAllocator(int blocks, int blockSize, ESFAllocator *source);

  /** Destructor.  If initialized, the allocator will call the its destroy
   *  method.  The destroy method may or may not return the allocator's
   *  managed memory to its source allocator.
   */
  virtual ~ESFFixedAllocator();

  /** Allocate a word-aligned memory block of at least size bytes.
   *
   *  @param size The minimum number of bytes to allocate.
   *  @return a word-aligned memory block of at least size bytes if
   *      successful, NULL otherwise.
   */
  virtual void *allocate(ESFUWord size);

  /** Deallocate a memory block allocated by this allocator or by its
   *  failover allocators.
   *
   *  @param block The block to deallocate
   *  @return ESF_SUCCESS if the block was successfully deallocated, another
   *      error code otherwise.  ESF_NOT_OWNER will be returned if the
   *      block was not allocated by this allocator.
   */
  virtual ESFError deallocate(void *block);

  /** Get the overhead in bytes of any additional control structures
   *  attached to an allocated block.  This can be used to optimize
   *  allocation requests for some allocators.  Power of two allocators,
   *  for example, will always round the size requested + the overhead up
   *  to the next power of two.  If the caller always requests powers of
   *  two, then the allocator can waste a lot of memory.  If, however, the
   *  caller requests a power of two minus the allocator overhead, then
   *  the allocator will return the optimal amount of memory.
   *
   *  @return the allocator's overhead in bytes for each allocation.
   */
  virtual ESFUWord getOverhead();

  /** Initialize this memory pool.
   *
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError initialize();

  /** Destroy this allocator.  The allocator will return all of the memory
   *  it manages to its source allocator and return itself to a state where
   *  initialize could be called again.  All memory should be returned to
   *  the allocator before calling its destroy method.   Some implementations
   *  may refuse to shutdown if they have outstanding allocations.
   *
   *  @return ESF_SUCCESS if the allocator could destroy itself, another
   *      error code otherwise.  ESF_IN_USE will be returned if the
   *      allocator has handed out memory that has not been returned.
   *  @see initialize.
   */
  virtual ESFError destroy();

  /** Get the allocator's current initialization state.
   *
   *  @return ESF_SUCCESS if the allocator is initialized,
   *      ESF_NOT_INITIALIZED if the allocator has not been initialized,
   *      another error code if an error occurred determining the
   *      allocator's current state.
   *  @see initialize.
   */
  virtual ESFError isInitialized();

  /** Set another allocator to be used if this allocator cannot fulfill a
   *  allocate request.  This failover allocator will be initialized lazily.
   *  If a failover allocator is not set, an allocator will simply return
   *  NULL if it cannot fulfill an allocation request.  Destroying an
   *  allocator with registered failover allocators must also destroy those
   *  failover allocators if they have been initialized.  Failover allocators
   *  may be chained.  An allocator is responsible for determining whether
   *  a block of memory to be deallocated came from itself or from its
   *  failover allocator and take the appropriate action.  Allocators should
   *  never be used as failover allocators for themselves to avoid
   *  infinite recursion.
   *
   *  @param allocator The failover allocator.  Set to NULL to clear an
   *      already registered failover allocator.
   *  @return ESF_SUCCESS if successful, another error code otherwise.
   */
  virtual ESFError setFailoverAllocator(ESFAllocator *allocator);

  /** Get the failover allocator used by this allocator.
   *
   *  @param allocator The failover allocator to get (pointer to a pointer).
   *      This pointer will be set to NULL if no failover allocator exists.
   *  @return ESF_SUCCESS if successful, another error code otherwise.  Note
   *      that if no failover allocator exists, ESF_SUCCESS will be returned
   *      and the allocator argument will be set to NULL.
   *  @see setFailoverAllocator
   */
  virtual ESFError getFailoverAllocator(ESFAllocator **allocator);

  /** Get the size of the blocks allocated by this allocator.
   *
   *  @return The block size of this allocator.
   */
  int getBlockSize();

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
  //  Disabled
  ESFFixedAllocator(const ESFFixedAllocator &);
  ESFFixedAllocator &operator=(const ESFFixedAllocator &);

  typedef struct AvailListElem {
    AvailListElem *_next;
  } AvailListElem;

  AvailListElem *popAvailList();
  void pushAvailList(AvailListElem *elem);

  AvailListElem *_availList;
  void *_pool;
  ESFAllocator *_sourceAllocator;
  ESFAllocator *_failoverAllocator;
  ESFUWord _blockSize;
  int _blocks;
};

#endif /* ! ESF_FIXED_ALLOCATOR_H */
