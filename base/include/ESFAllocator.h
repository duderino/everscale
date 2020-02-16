/** @file ESFAllocator.h
 *  @brief The interface for a generic memory allocator
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_ALLOCATOR_H
#define ESF_ALLOCATOR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

/** @defgroup allocator Memory Allocators
 */

/** The ESFAllocator defines the interface that any concrete allocator must
 *    realize.
 *
 *  @ingroup allocator
 */
class ESFAllocator {
public:
    /** Destructor.  If any memory allocated from this allocator has not
     *  been returned by the time it is destroyed, the allocator will not
     *  release its resources (better to leak memory than corrupt the heap).
     */
    virtual ~ESFAllocator() {
    }

    /** Allocate a word-aligned memory block of at least size bytes.
     *
     *    @param size The minimum number of bytes to allocate.
     *    @return a word-aligned memory block of at least size bytes if
     *        successful, NULL otherwise.
     */
    virtual void *allocate(ESFUWord size) = 0;

    /** Deallocate a memory block allocated by this allocator or by its
     *    failover allocators.
     *
     *    @param block The block to deallocate
     *    @return ESF_SUCCESS if the block was successfully deallocated, another
     *      error code otherwise.  ESF_NOT_OWNER will be returned if the
     *      block was not allocated by this allocator.
     */
    virtual ESFError deallocate(void *block) = 0;

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
    virtual ESFUWord getOverhead() = 0;

    /** Initialize this memory pool.
     *
     *    @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError initialize() = 0;

    /** Destroy this allocator.  The allocator will return all of the memory
     *  it manages to its source allocator and return itself to a state where
     *  initialize could be called again.  All memory should be returned to
     *  the allocator before calling its destroy method.   Some implementations
     *  may refuse to shutdown if they have outstanding allocations.
     *
     *    @return ESF_SUCCESS if the allocator could destroy itself, another
     *      error code otherwise.  ESF_IN_USE will be returned if the
     *      allocator has handed out memory that has not been returned.
     *    @see initialize.
     */
    virtual ESFError destroy() = 0;

    /** Get the allocator's current initialization state.
     *
     *  @return ESF_SUCCESS if the allocator is initialized,
     *      ESF_NOT_INITIALIZED if the allocator has not been initialized,
     *      another error code if an error occurred determining the
     *      allocator's current state.
     *  @see initialize.
     */
    virtual ESFError isInitialized() = 0;

    /** Set another allocator to be used if this allocator cannot fulfill a
     *    allocate request.  This failover allocator will be initialized lazily.
     *    If a failover allocator is not set, an allocator will simply return
     *    NULL if it cannot fulfill an allocation request.  Destroying an
     *    allocator with registered failover allocators must also destroy those
     *    failover allocators if they have been initialized.  Failover allocators
     *    may be chained.  An allocator is responsible for determining whether
     *    a block of memory to be deallocated came from itself or from its
     *    failover allocator and take the appropriate action.  Allocators should
     *  never be used as failover allocators for themselves to avoid
     *  infinite recursion.
     *
     *    @param allocator The failover allocator.  Set to NULL to clear an
     *        already registered failover allocator.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError setFailoverAllocator(ESFAllocator *allocator) = 0;

    /** Get the failover allocator used by this allocator.
     *
     *  @param allocator The failover allocator to get (pointer to a pointer).
     *      This pointer will be set to NULL if no failover allocator exists.
     *    @return ESF_SUCCESS if successful, another error code otherwise.  Note
     *      that if no failover allocator exists, ESF_SUCCESS will be returned
     *      and the allocator argument will be set to NULL.
     *    @see setFailoverAllocator
     */
    virtual ESFError getFailoverAllocator(ESFAllocator **allocator) = 0;
};

#endif /* ! ESF_ALLOCATOR_H */
