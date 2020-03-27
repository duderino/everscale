#ifndef ESB_SYSTEM_ALLOCATOR_H
#define ESB_SYSTEM_ALLOCATOR_H

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
   *  @param size The minimum number of bytes to allocate.
   *  @return a word-aligned memory block of at least size bytes if
   *      successful, NULL otherwise.
   */
  virtual void *allocate(UWord size);

  /** Deallocate a memory block allocated by this allocator or by its
   *  failover allocators.
   *
   *  @param block The block to deallocate.
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

 private:
  // Singleton
  SystemAllocator();
  //  Disabled
  SystemAllocator(const SystemAllocator &);
  SystemAllocator &operator=(const SystemAllocator &);

  AllocatorCleanupHandler _cleanupHandler;
  static SystemAllocator _Allocator;
};

}  // namespace ESB

#endif
