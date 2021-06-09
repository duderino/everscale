#ifndef ESB_DISCARD_ALLOCATOR_H
#define ESB_DISCARD_ALLOCATOR_H

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
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
   *  Any allocation larger than chunkSize will be passed to the source
   *  allocator.
   *
   *  @param chunkSize The size of the chunks the allocator uses.
   *  @param alignmentSize Returned allocation addresses will be a multiple of
   * this value, which must be a power of two.
   *  @param multipleOf The size of the allocations made against the
   * underlying allocator must be multiples of this.
   *  @param source The allocator to use to allocate any chunks.
   *        This is probably the system allocator.
   *  @see GetOverhead To determine how much extra memory the allocator
   *      will request from the source allocator for each chunk.
   */
  DiscardAllocator(UInt32 chunkSize, UInt16 alignmentSize = sizeof(Word), UInt16 multipleOf = 1,
                   Allocator &source = SystemAllocator::Instance(), bool forcePool = false);

  /** Destructor.  Any memory still used by the allocator will be return to
   *  the source allocator at this point.
   */
  virtual ~DiscardAllocator();

  /** Allocate a word-aligned memory block of at least size bytes.
   *
   * @param block will point to a word-aligned memory block of at least size bytes if successful, NULL otherwise.
   * @param size The minimum number of bytes to allocate.
   * @return ESB_SUCCESS if successful, ESB_OUT_OF_MEMORY if the allocator is exhausted, another error code otherwise.
   */
  virtual Error allocate(UWord size, void **block);

  /** No-op - memory will not be freed by the allocator until it is reset or destroyed.
   */
  virtual Error deallocate(void *block);

  /**
   * Determine whether the implementation supports reallocation.
   *
   * @return false
   */
  virtual bool reallocates();

  /**
   * Unsupported - this allocator does not support reallocation.
   */
  virtual Error reallocate(void *oldBlock, UWord size, void **newBlock);

  /**
   * Get a cleanup handler to free memory returned by this allocator.  The
   * lifetime of the cleanup handler is the lifetime of the allocator.
   *
   * @return A cleanup handler that can free memory allocated by this allocator.
   */
  virtual CleanupHandler &cleanupHandler();

  /** Free all chunks used by this allocator except for the first chunk.
   *  Mark the first chunk as available so it can be reused.
   *
   * @return ESB_SUCCESS if memory could be
   */
  Error reset();

  static inline ESB::UInt32 SizeofChunk(ESB::UInt32 alignmentSize) { return ESB_ALIGN(sizeof(Chunk), alignmentSize); }

 private:
  typedef struct Chunk {
    Chunk *_next;
    UWord _idx;
    UWord _size;
    char *_data;
  } Chunk;

  Error allocateChunk(int chunkSize, Chunk **chunk);

  Chunk *_head;
#ifdef ESB_NO_ALLOC
  bool _forcePool;
#endif
  UInt16 _alignmentSize;
  UInt16 _multipleOf;
  UInt32 _chunkSize;
  Allocator &_source;
  AllocatorCleanupHandler _cleanupHandler;

  ESB_DEFAULT_FUNCS(DiscardAllocator);
};

}  // namespace ESB

#endif
