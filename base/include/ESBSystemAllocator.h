#ifndef ESB_SYSTEM_ALLOCATOR_H
#define ESB_SYSTEM_ALLOCATOR_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** SystemAllocator realizes the Allocator interface by calling the
 *  system's global memory allocation routines (e.g., malloc and free).
 *
 *  @ingroup allocator
 */
class SystemAllocator : public Allocator {
 public:
  static SystemAllocator &Instance();

  /** Destructor.  If any memory allocated from this allocator has not
   *  been returned by the time it is destroyed, the allocator will not
   *  release its resources (better to leak memory than corrupt the heap).
   */
  virtual ~SystemAllocator();

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
   * @return ESB_SUCCESS if the block was successfully deallocated, another error code otherwise.
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

 private:
  // Singleton
  SystemAllocator();

  AllocatorCleanupHandler _cleanupHandler;
  static SystemAllocator _Allocator;

  ESB_DEFAULT_FUNCS(SystemAllocator);
};

}  // namespace ESB

#endif
