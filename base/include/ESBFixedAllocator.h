#ifndef ESB_FIXED_ALLOCATOR_H
#define ESB_FIXED_ALLOCATOR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ESB {

/** FixedAllocator realizes the Allocator interface with a simple fixed
 *  length allocator.  The FixedAllocator handles memory reservation and
 *  compaction very efficiently with the low overhead of 1 word per
 *  allocation because of its limitation that all allocations are always the
 *  same, fixed size.
 *
 *  @ingroup allocator
 */
class FixedAllocator : public Allocator {
 public:
  /** Constructor.  Creates the FixedAllocator given the number of blocks
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
  FixedAllocator(UInt32 blocks, UInt32 blockSize, Allocator &source = SystemAllocator::Instance());

  /** Destructor.  If initialized, the allocator will call the its destroy
   *  method.  The destroy method may or may not return the allocator's
   *  managed memory to its source allocator.
   */
  virtual ~FixedAllocator();

  /** Allocate a word-aligned memory block of at least size bytes.
   *
   *  @param size The minimum number of bytes to allocate.
   *  @return a word-aligned memory block of at least size bytes if
   *      successful, NULL otherwise.
   */
  virtual void *allocate(UWord size);

  /** Deallocate a memory block allocated by this allocator or by its
   *  failover allocators.
   *
   *  @param block The block to deallocate
   *  @return ESB_SUCCESS if the block was successfully deallocated, another
   *      error code otherwise.  ESB_NOT_OWNER will be returned if the
   *      block was not allocated by this allocator.
   */
  virtual Error deallocate(void *block);

  /**
   * Determine whether the implementation supports reallocation.
   *
   * @return false
   */
  virtual bool reallocates();

  /**
   * Reallocate a block of memory or create a new block of memory if necessary.  Regardless, the contents of the
   * original block will be present in the returned block.
   *
   * @param block The block to reallocate
   * @param size
   * @return a word-aligned memory block of at least size bytes if successful, NULL otherwise.
   */
  virtual void *reallocate(void *block, UWord size);

  /**
   * Get a cleanup handler to free memory returned by this allocator.  The
   * lifetime of the cleanup handler is the lifetime of the allocator.
   *
   * @return A cleanup handler that can free memory allocated by this allocator.
   */
  virtual CleanupHandler &cleanupHandler();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  Error initialize();
  Error destroy();

  typedef struct AvailListElem {
    AvailListElem *_next;
  } AvailListElem;

  AvailListElem *popAvailList();
  void pushAvailList(AvailListElem *elem);

  AvailListElem *_availList;
  void *_pool;
  Allocator &_sourceAllocator;
  UWord _blockSize;
  UInt32 _blocks;
  AllocatorCleanupHandler _cleanupHandler;

  ESB_DISABLE_AUTO_COPY(FixedAllocator);
};

}  // namespace ESB

#endif
