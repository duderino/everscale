#ifndef ESB_BUDDY_ALLOCATOR_H
#define ESB_BUDDY_ALLOCATOR_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** BuddyAllocator realizes the Allocator interface with an implementation of the Buddy System algorithm described in
 *  Donald Knuth's The Art of Computer Programming, Volume 1 Fundamental Algorithms Third Edition.  It's a variable
 *  length allocator with a good algorithm for compacting reclaimed memory.
 *
 *  @ingroup allocator
 */
class BuddyAllocator : public Allocator {
 public:
  /** Constructor.
   *
   *  @param size The size of the memory pool that the cache will manage (will be rounded up to a power of 2).
   *  @param source The allocator to use to allocate the memory pool. This is probably the system allocator.
   */
  BuddyAllocator(UInt32 size, Allocator &source);

  /** Destructor.  The allocator will leak memory instead of risking a double free if there are any outstanding
   *  allocations.
   */
  virtual ~BuddyAllocator();

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
   * @param size The requested size of the new block
   * @param newBlock will point to a word-aligned memory block of at least size bytes if successful, NULL otherwise. Any
   * bytes stored in the oldBlock will be present at the start of the new block.
   * @return ESB_SUCCESS if successful, ESB_OUT_OF_MEMORY if the allocator is exhausted (in which case the oldBlock will
   * be left untouched), another error code otherwise.
   */
  virtual Error reallocate(void *oldBlock, UWord size, void **newBlock);

  /**
   * Get a cleanup handler to free memory returned by this allocator.  The
   * lifetime of the cleanup handler is the lifetime of the allocator.
   *
   * @return A cleanup handler that can free memory allocated by this allocator.
   */
  virtual CleanupHandler &cleanupHandler();

  /**
   * Determine the size of the allocators per-allocation book keeping.
   *
   * @return The per-allocation overhead of the allocator.
   */
  static inline Size Overhead() { return sizeof(AvailListElem); }

  /**
   * Release all memory used by the allocator if none is in use.  Note that the destructor calls this automatically.
   *
   * @return ESB_SUCCESS if successful, ESB_IN_USE if any memory previously allocated by this allocator has not been
   * returned.
   */
  Error reset();

 protected:
  /**
   * Get the size of the allocation.  This may be a bit larger than originally requested, but it can be assumed that the
   * block owns this much space until it is deallocated.
   *
   * @param block The block
   * @return 0 if the block was not allocated by this allocator, the block's size otherwise.
   */
  UWord allocationSize(void *block) const;

 private:
  Error initialize();

#ifdef ESB_64BIT
#define ESB_AVAIL_LIST_LENGTH 64
#elif defined ESB_32BIT
#define ESB_AVAIL_LIST_LENGTH 32
#else
#error "Cannot determine architecture type"
#endif

  typedef UInt8 KVal;
  typedef UInt8 Tag;

  typedef struct AvailListElem {
    AvailListElem *_linkB; /**< Backward link */
    AvailListElem *_linkF; /**< Forward link */
    KVal _kVal;            /**< k where 2^k is the size of this element. */
    Tag _tag;              /**< 1 if available, 0 if in use. */
#ifdef ESB_64BIT
    UInt8 _pad[6]; /**< keep the following data offset word aligned */
#else
    UInt8 _pad[2]; /**< keep the following data offset word aligned */
#endif
  } AvailListElem;

  /**
   * Round an unsigned word up to the nearest power of 2.
   *
   * @param uWord The unsigned word to round up
   * @return the nearest power of 2
   */
  static KVal UWordToKVal(UWord uWord);

  static inline UWord KValToUWord(KVal kVal) { return (ESB_UWORD_C(1) << kVal); }

  static inline KVal AdjustedKVal(UWord requestedSize) { return UWordToKVal(requestedSize + sizeof(AvailListElem)); }

  inline bool owns(void *block) const {
    char *lowerBound = (char *)_pool;
    char *upperBound = ((char *)_pool) + (ESB_UWORD_C(1) << _poolKVal);
    return (block < lowerBound || block >= upperBound) ? false : true;
  }

  AvailListElem *popAvailList(KVal kVal);
  void pushAvailList(AvailListElem *elem);
  void removeFromAvailList(AvailListElem *elem);

  AvailListElem *_availList[ESB_AVAIL_LIST_LENGTH];
  Allocator &_sourceAllocator;
  void *_pool;
  UWord _poolKVal;
  AllocatorCleanupHandler _cleanupHandler;

  ESB_DEFAULT_FUNCS(BuddyAllocator);
};

}  // namespace ESB

#endif
