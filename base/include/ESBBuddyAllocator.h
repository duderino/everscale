#ifndef ESB_BUDDY_ALLOCATOR_H
#define ESB_BUDDY_ALLOCATOR_H

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

/** BuddyAllocator realizes the Allocator interface with an
 *  implementation of the Buddy System algorithm described in Donald
 *  Knuth's The Art of Computer Programming, Volume 1 Fundamental Algorithms
 *  Third Edition.  It's a variable length allocator with a good algorithm
 *  for compacting reclaimed memory.
 *
 *  The overhead of this allocator breaks down as follows:
 *
 *  MaxPoolSize = 32 for 32-bit architectures, 64 for 64-bit.
 *  MaxPoolSizeInBytes = 2^MaxPoolSize
 *  WordSize = 4 bytes for 32-bit architectures, 8 for 64-bit.
 *
 *  AdjustedSize = RequestedSize + ( 3 * WordSize )
 *  AllocatedSize = Smallest power of 2 sufficient to hold AdjustedSize
 *
 *  Fixed Overhead:
 *
 *    ( MaxPoolSize * WordSize ) + ( WordSize * 4 )
 *
 *  Overhead Per Allocation:
 *
 *    AllocatedSize - RequestedSize.
 *
 *  @ingroup allocator
 */
class BuddyAllocator : public Allocator {
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
  BuddyAllocator(UInt32 size, Allocator &source);

  /** Destructor.  If initialized, the allocator will call the its destroy
   *  method.  The destroy method may or may not return the allocator's
   *  managed memory to its source allocator.
   */
  virtual ~BuddyAllocator();

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
  // Disabled
  BuddyAllocator(const BuddyAllocator &);
  BuddyAllocator &operator=(const BuddyAllocator &);

  Error initialize();
  Error destroy();

#ifdef ESB_64BIT
#define ESB_AVAIL_LIST_LENGTH 64
  typedef UInt32 KVal;
  typedef UInt32 Tag;
#elif defined ESB_32BIT
#define ESB_AVAIL_LIST_LENGTH 32
  typedef UInt16 KVal;
  typedef UInt16 Tag;
#else
#error "Cannot determine architecture type"
#endif

  typedef struct AvailListElem {
    AvailListElem *_linkB; /**< Backward link */
    AvailListElem *_linkF; /**< Forward link */
    KVal _kVal;            /**< k where 2^k is the size of this element. */
    Tag _tag;              /**< 1 if available, 0 if in use. */
  } AvailListElem;

  static KVal GetKVal(UWord requestedSize);
  AvailListElem *popAvailList(KVal kVal);
  void pushAvailList(AvailListElem *elem);
  void removeFromAvailList(AvailListElem *elem);

  AvailListElem *_availList[ESB_AVAIL_LIST_LENGTH];
  Allocator &_sourceAllocator;
  void *_pool;
  UWord _poolKVal;
  AllocatorCleanupHandler _cleanupHandler;
};

}  // namespace ESB

#endif
