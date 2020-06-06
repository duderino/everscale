#ifndef ESB_SHARED_ALLOCATOR_H
#define ESB_SHARED_ALLOCATOR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

namespace ESB {

/** SharedAllocator is a decorator that can synchronize access
 *  to any other allocator with a mutex.
 *
 *  @ingroup allocator
 */
class SharedAllocator : public Allocator {
 public:
  /** Constructor.
   *
   *  @param allocator The decorated allocator to synchronize.
   */
  SharedAllocator(Allocator &allocator);

  /** Destructor.  If initialized, the allocator will call the its destroy
   *  method.  The destroy method may or may not return the allocator's
   *  managed memory to its source allocator.
   */
  virtual ~SharedAllocator();

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
   *  @param block The block to deallocate (pointer to a pointer).  If
   *      freed, the block pointer will be set to NULL.
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
  //  Disabled
  SharedAllocator(const SharedAllocator &);
  SharedAllocator &operator=(const SharedAllocator &);

  Allocator &_allocator;
  Mutex _mutex;
  AllocatorCleanupHandler _sharedCleanupHandler;
};

}  // namespace ESB

#endif
