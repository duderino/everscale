/** @file ESFDiscardAllocator.h
 *  @brief An allocator good for lots of small allocations with the same
 *      lifetime.
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

#ifndef ESF_DISCARD_ALLOCATOR_H
#define ESF_DISCARD_ALLOCATOR_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

/** An allocator good for lots of small allocations with the same lifetime.
 *  No memory is actually freed by the allocator until it is destroyed.
 *  ESFDiscardAllocator is a very simple allocator.  It allocates
 *  memory from its source allocator one chunk as a time.  As it hands out
 *  memory, it advances a pointer into the current chunk, making sure that
 *  the resulting allocation would be word-aligned.  If there is not enough
 *  memory left in the current chunk, it allocates a new chunk.
 *  <p>
 *  What ESFDiscardAllocator does not do is handle deallocation.  Its
 *  deallocate methods are essentially no-ops.  Since it does not have to
 *  handle deallocations, it does not have to attach any additional control
 *  structures to the memory it hands out.  Memory is instead returned to the
 *  source allocator all at once, either when the allocator goes out of scope
 *  or when explicitly released by the application.
 *  </p>
 *  <p>
 *  The result is an allocator that is good for many small allocations with the
 *  same lifetime.  Large allocations can cause the allocator to
 *  waste memory and allocate chunks too early from the source allocator.
 *  Since memory for the allocator can only be reused after the last allocation
 *  is "finished", a single allocation can prevent all other
 *  allocations from being reused.  The allocator works best for allocations
 *  that have lifespans close to each other.
 *  </p>
 *  <p>
 *  Note well, the ESFDiscardAllocator does not call the destructors of any
 *  object allocated with its memory.  If destructors need to be called,
 *  it is the application's responsibility to manually call objects'
 *  destructors before explicitly releasing the allocator's memory or allowing
 *  the allocator to go out of scope.
 *  </p>
 *
 *  @ingroup allocator
 */
class ESFDiscardAllocator: public ESFAllocator {
public:

    /** Constructor.  The allocator will request memory from the source
     *  allocator in chunkSize + overhead sized chunks.  The overhead reflects
     *  the allocator's own internal state that it needs to manage each chunk.
     *  Any allocation larger than chunkSize will be passed to the failover
     *  allocator if one has been configured.
     *
     *  @param chunkSize The size of the chunks the allocator uses.
     *  @param source The allocator to use to allocate any chunks.
     *        This is probably the system allocator.
     *  @see GetOverhead To determine how much extra memory the allocator
     *      will request from the source allocator for each chunk.
     */
    ESFDiscardAllocator(int chunkSize, ESFAllocator *source);

    /** Constructor.
     */
    ESFDiscardAllocator();

    /** Initializer.  The allocator will request memory from the source
     *  allocator in chunkSize + overhead sized chunks.  The overhead reflects
     *  the allocator's own internal state that it needs to manage each chunk.
     *  Any allocation larger than chunkSize will be passed to the failover
     *  allocator if one has been configured.
     *
     *  @param chunkSize The size of the chunks the allocator uses.
     *  @param source The allocator to use to allocate any chunks.
     *        This is probably the system allocator.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    ESFError initialize(int chunkSize, ESFAllocator *source);

    /** Destructor.  Any memory still used by the allocator will be return to
     *  the source allocator at this point.
     */
    virtual ~ESFDiscardAllocator();

    /** Allocate a word-aligned memory block of at least size bytes.
     *
     *  @param size The minimum number of bytes to allocate.  If the size
     *      exceeds the chunkSize argument passed to the constructor, this
     *      allocation will be passed to the failover allocator if set or
     *      rejected if not set.
     *  @return a word-aligned memory block of at least size bytes if
     *      successful, NULL otherwise.
     */
    virtual void *allocate(ESFUWord size);

    /** Deallocate a memory block allocated by this allocator.  This is
     *  a no-op and need not be called unless its possible another
     *  allocator might one day be used by the same code (using the
     *  ESFAllocator interface).
     *
     *  @param block The block to deallocate
     *  @return ESF_SUCCESS if the block was successfully deallocated, another
     *      error code otherwise.  ESF_NOT_OWNER will be returned if the
     *      block was not allocated by this allocator.
     */
    virtual ESFError deallocate(void *block);

    /** Get the overhead in bytes of any additional control structures
     *  attached to an allocated chunk.  This can be used to optimize
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

    /** Free all chunks used by this allocator except for the first chunk.
     *  Mark the first chunk as available so it can be reused.
     *
     * @return ESF_SUCCESS if memory could be
     */
    ESFError reset();

    /** Get the allocators current initialization state.
     *
     *  @return ESF_SUCCESS if the allocator is initialized,
     *      ESF_NOT_INITIALIZED if the allocator has not been initialized,
     *      another error code if an error occurred determining the
     *      allocator's current state.
     *  @see initialize.
     */
    virtual ESFError isInitialized();

    /** This is a no-op.  The allocator passed to the constructor is always
     *  used to allocate chunks.
     *
     *  @param allocator The failover allocator.  Set to NULL to clear an
     *      already registered failover allocator.
     *  @return ESF_SUCCESS if successful, another error code otherwise.
     */
    virtual ESFError setFailoverAllocator(ESFAllocator *allocator);

    /** Get the failover allocator used by this allocator.  This always
     *  returns the allocator passed to the constructor.
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

    /** Get the size of the extra overhead the allocator adds to every chunk
     *  allocation.
     *
     *  @return The size in bytes of the allocator's overhead per chunk.
     */
    static ESFSize GetOverheadSize();

private:

    //  Disabled
    ESFDiscardAllocator(const ESFDiscardAllocator &);
    ESFDiscardAllocator &operator=(const ESFDiscardAllocator &);

    typedef struct Chunk {
        Chunk *_next;
        ESFUWord _idx;
        ESFUWord _size;
        char *_data;
    } Chunk;

    Chunk *allocateChunk(int chunkSize);

    Chunk *_head;

    ESFUWord _chunkSize;
    ESFAllocator *_source;
};

#endif /* ! ESF_DISCARD_ALLOCATOR_H */
