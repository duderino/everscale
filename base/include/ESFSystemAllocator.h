/** @file ESFSystemAllocator.h
 *  @brief An implementation of the ESFAllocator interface that uses the
 *      system's heap
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SYSTEM_ALLOCATOR_H
#define ESF_SYSTEM_ALLOCATOR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

/** ESFSystemAllocator realizes the ESFAllocator interface by calling the
 *    system's global memory allocation routines (e.g., malloc and free).
 *
 *  @ingroup allocator
 */
class ESFSystemAllocator: public ESFAllocator {
public:
    static ESFSystemAllocator *GetInstance();

    /** Destructor.  If any memory allocated from this allocator has not
     *  been returned by the time it is destroyed, the allocator will not
     *  release its resources (better to leak memory than corrupt the heap).
     */
    virtual ~ESFSystemAllocator();

    /** Allocate a word-aligned memory block of at least size bytes.
     *
     *  @param size The minimum number of bytes to allocate.
     *  @return a word-aligned memory block of at least size bytes if
     *      successful, NULL otherwise.
     */
    virtual void *allocate(ESFUWord size);

    /** Allocate a word-aligned memory block of at least size bytes.
     *
     *  @param block The block to allocate (pointer to a pointer)
     *  @param size The minimum number of bytes to allocate.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError allocate(void **block, ESFUWord size);

    /** Deallocate a memory block allocated by this allocator or by its
     *  failover allocators.
     *
     *  @param block The block to deallocate.
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

    /** Initialize this memory pool.  This operation does not need to be
     *  called on the ESFSystemAllocator, the ESFSystemAllocator is always
     *  in the initialized state.
     *
     *  @return ESF_SUCCESS.
     */
    virtual ESFError initialize();

    /** Destroy this allocator.  This operation is not supported, the
     *  ESFSystemAllocator is always in the initialized state.
     *
     *  @return ESF_OPERATION_NOT_SUPPORTED.
     *  @see initialize.
     */
    virtual ESFError destroy();

    /** Get the allocators current initialization state.  The
     *  ESFSystemAllocator is always in the initialized state.
     *
     *  @return ESF_SUCCESS.
     *  @see initialize.
     */
    virtual ESFError isInitialized();

    /** Set another allocator to be used if this allocator cannot fulfill a
     *  allocate request.  The ESFSystemAllocator does not support failover
     *  allocators (it cannot distinguish between blocks it allocates and
     *  blocks its failover allocator allocates).
     *
     *  @param allocator The failover allocator.  Set to NULL to clear an
     *      already registered failover allocator.
     *  @return ESF_OPERATION_NOT_SUPPORTED
     */
    virtual ESFError setFailoverAllocator(ESFAllocator *allocator);

    /** Get the failover allocator used by this allocator.  The
     *  ESFSystemAllocator does not support failover allocators (it cannot
     *  distinguish between blocks it allocates and blocks its failover
     *  allocator allocates).
     *
     *  @param allocator The failover allocator to get (pointer to a pointer).
     *  @return ESF_OPERATION_NOT_SUPPORTED.
     */
    virtual ESFError getFailoverAllocator(ESFAllocator **allocator);

private:
    //  Disabled
    ESFSystemAllocator();
    ESFSystemAllocator(const ESFSystemAllocator &);
    ESFSystemAllocator &operator=(const ESFSystemAllocator &);

    static ESFSystemAllocator _Allocator;
};

#endif /* ! ESF_SYSTEM_ALLOCATOR_H */
