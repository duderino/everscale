/** @file ESFBuddyAllocator.h
 *  @brief A ESFAllocator implementation good for variable-length allocations
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

#ifndef ESF_BUDDY_ALLOCATOR_H
#define ESF_BUDDY_ALLOCATOR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

/** ESFBuddyAllocator realizes the ESFAllocator interface with an
 *    implementation of the Buddy System algorithm described in Donald
 *    Knuth's The Art of Computer Programming, Volume 1 Fundamental Algorithms
 *    Third Edition.  It's a variable length allocator with a good algorithm
 *    for compacting reclaimed memory.
 *
 *    The overhead of this allocator breaks down as follows:
 *
 *    MaxPoolSize = 32 for 32-bit architectures, 64 for 64-bit.
 *    MaxPoolSizeInBytes = 2^MaxPoolSize
 *    WordSize = 4 bytes for 32-bit architectures, 8 for 64-bit.
 *
 *    AdjustedSize = RequestedSize + ( 3 * WordSize )
 *    AllocatedSize = Smallest power of 2 sufficient to hold AdjustedSize
 *
 *    Fixed Overhead:
 *
 *        ( MaxPoolSize * WordSize ) + ( WordSize * 4 )
 *
 *    Overhead Per Allocation:
 *
 *        AllocatedSize - RequestedSize.
 *
 *  @ingroup allocator
 */
class ESFBuddyAllocator: public ESFAllocator {
public:

    /** Constructor.
     *
     *    @param size The size of the memory pool that this allocator will
     *        manage as a power of 2 (e.g., 16 will create a 2^16 byte pool).
     *        Note that this pool will not be allocated until the initialize
     *        method is called.
     *    @param source The allocator to use to allocate the memory pool.
     *        This is probably the system allocator.
     */
    ESFBuddyAllocator(int size, ESFAllocator *source);

    /** Destructor.  If initialized, the allocator will call the its destroy
     *  method.  The destroy method may or may not return the allocator's
     *  managed memory to its source allocator.
     */
    virtual ~ESFBuddyAllocator();

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

    // Disabled
    ESFBuddyAllocator(const ESFBuddyAllocator &);
    ESFBuddyAllocator &operator=(const ESFBuddyAllocator &);

#ifdef ESF_64BIT

#define ESF_AVAIL_LIST_LENGTH 64

    typedef ESFUInt32 KVal;
    typedef ESFUInt32 Tag;

#elif defined ESF_32BIT

#define ESF_AVAIL_LIST_LENGTH 32

    typedef ESFUInt16 KVal;
    typedef ESFUInt16 Tag;

#else

#error "Cannot determine architecture type"

#endif

    typedef struct AvailListElem {
        AvailListElem *_linkB; /**< Backward link */
        AvailListElem *_linkF; /**< Forward link */
        KVal _kVal; /**< k where 2^k is the size of this element. */
        Tag _tag; /**< 1 if available, 0 if in use. */
    } AvailListElem;

    static KVal GetKVal(ESFUWord requestedSize);
    AvailListElem *popAvailList(KVal kVal);
    void pushAvailList(AvailListElem *elem);
    void removeFromAvailList(AvailListElem *elem);

    AvailListElem *_availList[ESF_AVAIL_LIST_LENGTH];

    ESFAllocator *_failoverAllocator;
    ESFAllocator *_sourceAllocator;
    char *_pool;
    ESFUWord _poolKVal;
};

#endif /* ! ESF_BUDDY_ALLOCATOR_H */
