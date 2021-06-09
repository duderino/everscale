#ifndef ESB_SHARED_ALLOCATOR_H
#define ESB_SHARED_ALLOCATOR_H

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
   * @param block will point to a word-aligned memory block of at least size bytes if successful, NULL otherwise.
   * @param size The minimum number of bytes to allocate.
   * @return ESB_SUCCESS if successful, ESB_OUT_OF_MEMORY if the allocator is exhausted, another error code otherwise.
   */
  virtual Error allocate(UWord size, void **block);

  /** Deallocate a memory block allocated by this allocator.
   *
   * @param block The block to deallocate
   * @return ESB_SUCCESS if the block was successfully deallocated, another error code otherwise.  ESB_NOT_OWNER may be
   * returned if the block was not allocated by this allocator, but not all implementations support this.
   */
  virtual Error deallocate(void *block);

  /**
   * Determine whether the implementation supports reallocation.
   *
   * @return true if the implementation supports reallocation, false otherwise.
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

 private:
  Allocator &_allocator;
  Mutex _mutex;
  AllocatorCleanupHandler _sharedCleanupHandler;

  ESB_DEFAULT_FUNCS(SharedAllocator);
};

}  // namespace ESB

#endif
