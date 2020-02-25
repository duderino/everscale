#ifndef ESB_DISCARD_ALLOCATOR_H
#define ESB_DISCARD_ALLOCATOR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ESB {

/** An allocator good for lots of small allocations with the same lifetime.
 *  No memory is actually freed by the allocator until it is destroyed.
 *  DiscardAllocator is a very simple allocator.  It allocates
 *  memory from its source allocator one chunk as a time.  As it hands out
 *  memory, it advances a pointer into the current chunk, making sure that
 *  the resulting allocation would be word-aligned.  If there is not enough
 *  memory left in the current chunk, it allocates a new chunk.
 *  <p>
 *  What DiscardAllocator does not do is handle deallocation.  Its
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
 *  Note well, the DiscardAllocator does not call the destructors of any
 *  object allocated with its memory.  If destructors need to be called,
 *  it is the application's responsibility to manually call objects'
 *  destructors before explicitly releasing the allocator's memory or allowing
 *  the allocator to go out of scope.
 *  </p>
 *
 *  @ingroup allocator
 */
class DiscardAllocator : public Allocator {
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
  DiscardAllocator(int chunkSize, Allocator *source);

  /** Constructor.
   */
  DiscardAllocator();

  /** Initializer.  The allocator will request memory from the source
   *  allocator in chunkSize + overhead sized chunks.  The overhead reflects
   *  the allocator's own internal state that it needs to manage each chunk.
   *  Any allocation larger than chunkSize will be passed to the failover
   *  allocator if one has been configured.
   *
   *  @param chunkSize The size of the chunks the allocator uses.
   *  @param source The allocator to use to allocate any chunks.
   *        This is probably the system allocator.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  Error initialize(int chunkSize, Allocator *source);

  /** Destructor.  Any memory still used by the allocator will be return to
   *  the source allocator at this point.
   */
  virtual ~DiscardAllocator();

  /** Allocate a word-aligned memory block of at least size bytes.
   *
   *  @param size The minimum number of bytes to allocate.  If the size
   *      exceeds the chunkSize argument passed to the constructor, this
   *      allocation will be passed to the failover allocator if set or
   *      rejected if not set.
   *  @return a word-aligned memory block of at least size bytes if
   *      successful, NULL otherwise.
   */
  virtual void *allocate(UWord size);

  /** Deallocate a memory block allocated by this allocator.  This is
   *  a no-op and need not be called unless its possible another
   *  allocator might one day be used by the same code (using the
   *  Allocator interface).
   *
   *  @param block The block to deallocate
   *  @return ESB_SUCCESS if the block was successfully deallocated, another
   *      error code otherwise.  ESB_NOT_OWNER will be returned if the
   *      block was not allocated by this allocator.
   */
  virtual Error deallocate(void *block);

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
  virtual UWord getOverhead();

  /** Initialize this memory pool.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error initialize();

  /** Destroy this allocator.  The allocator will return all of the memory
   *  it manages to its source allocator and return itself to a state where
   *  initialize could be called again.  All memory should be returned to
   *  the allocator before calling its destroy method.   Some implementations
   *  may refuse to shutdown if they have outstanding allocations.
   *
   *  @return ESB_SUCCESS if the allocator could destroy itself, another
   *      error code otherwise.  ESB_IN_USE will be returned if the
   *      allocator has handed out memory that has not been returned.
   *  @see initialize.
   */
  virtual Error destroy();

  /** Free all chunks used by this allocator except for the first chunk.
   *  Mark the first chunk as available so it can be reused.
   *
   * @return ESB_SUCCESS if memory could be
   */
  Error reset();

  /** Get the allocators current initialization state.
   *
   *  @return ESB_SUCCESS if the allocator is initialized,
   *      ESB_NOT_INITIALIZED if the allocator has not been initialized,
   *      another error code if an error occurred determining the
   *      allocator's current state.
   *  @see initialize.
   */
  virtual Error isInitialized();

  /** This is a no-op.  The allocator passed to the constructor is always
   *  used to allocate chunks.
   *
   *  @param allocator The failover allocator.  Set to NULL to clear an
   *      already registered failover allocator.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error setFailoverAllocator(Allocator *allocator);

  /** Get the failover allocator used by this allocator.  This always
   *  returns the allocator passed to the constructor.
   *
   *  @param allocator The failover allocator to get (pointer to a pointer).
   *      This pointer will be set to NULL if no failover allocator exists.
   *  @return ESB_SUCCESS if successful, another error code otherwise.  Note
   *      that if no failover allocator exists, ESB_SUCCESS will be returned
   *      and the allocator argument will be set to NULL.
   *  @see setFailoverAllocator
   */
  virtual Error getFailoverAllocator(Allocator **allocator);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

  /** Get the size of the extra overhead the allocator adds to every chunk
   *  allocation.
   *
   *  @return The size in bytes of the allocator's overhead per chunk.
   */
  static Size GetOverheadSize();

 private:
  //  Disabled
  DiscardAllocator(const DiscardAllocator &);
  DiscardAllocator &operator=(const DiscardAllocator &);

  typedef struct Chunk {
    Chunk *_next;
    UWord _idx;
    UWord _size;
    char *_data;
  } Chunk;

  Chunk *allocateChunk(int chunkSize);

  Chunk *_head;

  UWord _chunkSize;
  Allocator *_source;
};

}  // namespace ESB

#endif
