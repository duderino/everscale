#ifndef ESB_BUDDY_CACHE_ALLOCATOR_H
#define ESB_BUDDY_CACHE_ALLOCATOR_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_BUDDY_ALLOCATOR_H
#include <ESBBuddyAllocator.h>
#endif

namespace ESB {

/** BuddyCacheAllocator uses a BuddyAllocator as a cache in front of another
 *  Allocator.  When the BuddyAllocator is exhausted, allocations will instead
 *  failover to the other allocator, and deallocations will be returned to
 *  the appropriate allocator.
 */
class BuddyCacheAllocator : public BuddyAllocator {
 public:
  /** Constructor.
   *
   *  @param size The size of the memory pool that the cache will manage (will be rounded up to a power of 2).
   *  @param source The allocator to use to allocate the memory pool. This is probably the system allocator
   *  @param failover The allocator that will be used when the cache is exhausted.
   */
  BuddyCacheAllocator(UInt32 size, Allocator &source, Allocator &failover);

  /** Destructor.  The cache will leak memory instead of risking a double free if there are any outstanding allocations.
   */
  virtual ~BuddyCacheAllocator();

  /** Allocate a word-aligned memory block of at least size bytes.
   *
   * @param block will point to a word-aligned memory block of at least size bytes if successful, NULL otherwise.
   * @param size The minimum number of bytes to allocate.
   * @return ESB_SUCCESS if successful, ESB_OUT_OF_MEMORY if the allocator is exhausted, another error code otherwise.
   */
  virtual Error allocate(UWord size, void **block);

  /** Deallocate a memory block allocated by this allocator.
   *
   * @param block The block to deallocate
   * @return ESB_SUCCESS if the block was successfully deallocated, another error code otherwise.  ESB_NOT_OWNER will be
   * returned if the block was not allocated by this allocator.
   */
  virtual Error deallocate(void *block);

  /**
   * Determine whether the implementation supports reallocation.
   *
   * @return true
   */
  virtual bool reallocates();

  /**
   * Reallocate a block of memory or create a new block of memory if necessary.  The contents of the original block will
   * be present in the returned block.
   *
   * @param oldBlock The block to reallocate
   * @param newBlockSize The requested newBlockSize of the new block
   * @param newBlock will point to a word-aligned memory block of at least newBlockSize bytes if successful, NULL
   * otherwise. Any bytes stored in the oldBlock will be present at the start of the new block.
   * @return ESB_SUCCESS if successful, ESB_OUT_OF_MEMORY if the allocator is exhausted (in which case the oldBlock will
   * be left untouched), another error code otherwise.
   */
  virtual Error reallocate(void *oldBlock, UWord newBlockSize, void **newBlock);

  /**
   * Get the sum of all allocation sizes made from the cache
   *
   * @return sum of allocation sizes from the cache
   */
  inline UInt64 cacheBytes() const { return _cacheBytes; }

  /**
   * Get the sum of all allocation sizes made from the failover allocator
   *
   * @return sum of allocation sizes from the failover allocator
   */
  inline UInt64 failoverBytes() const { return _failoverBytes; }

 private:
  UInt64 _cacheBytes;
  UInt64 _failoverBytes;
  Allocator &_failover;

  ESB_DEFAULT_FUNCS(BuddyCacheAllocator);
};

}  // namespace ESB

#endif
