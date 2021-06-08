#ifndef ESB_ALLOCATOR_H
#define ESB_ALLOCATOR_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_CLEANUP_HANDLER_H
#include <ESBCleanupHandler.h>
#endif

namespace ESB {

/** @defgroup allocator Memory Allocators
 */

/** The interface for memory allocators.
 *
 *  @ingroup allocator
 */
class Allocator {
 public:
  /** Constructor
   */
  Allocator();

  /** Destructor
   */
  virtual ~Allocator();

  /** Allocate a word-aligned memory block of at least size bytes.
   *
   * @param size The minimum number of bytes to allocate.
   * @return a word-aligned memory block of at least size bytes if successful, NULL otherwise.
   */
  virtual void *allocate(UWord size) = 0;

  /** Deallocate a memory block allocated by this allocator or by its failover allocators.
   *
   * @param block The block to deallocate
   * @return ESB_SUCCESS if the block was successfully deallocated, another error code otherwise.  ESB_NOT_OWNER will be
   * returned if the block was not allocated by this allocator.
   */
  virtual Error deallocate(void *block) = 0;

  /**
   * Determine whether the implementation supports reallocation.
   *
   * @return true if the implementation supports reallocation, false otherwise.
   */
  virtual bool reallocates() = 0;

  /**
   * Reallocate a block of memory or create a new block of memory if necessary.  Regardless, the contents of the
   * original block will be present in the returned block.
   *
   * @param block The block to reallocate
   * @param size
   * @return a word-aligned memory block of at least size bytes if successful, NULL otherwise.
   */
  virtual void *reallocate(void *block, UWord size) = 0;

  /**
   * Get a cleanup handler to free memory returned by this allocator.  The
   * lifetime of the cleanup handler is the lifetime of the allocator.
   *
   * @return A cleanup handler that can free memory allocated by this allocator.
   */
  virtual CleanupHandler &cleanupHandler() = 0;

  ESB_DISABLE_AUTO_COPY(Allocator);
};

class AllocatorCleanupHandler : public CleanupHandler {
 public:
  AllocatorCleanupHandler(Allocator &allocator);
  virtual ~AllocatorCleanupHandler();
  virtual void destroy(Object *object);

 private:
  Allocator &_allocator;

  ESB_DISABLE_AUTO_COPY(AllocatorCleanupHandler);
};

}  // namespace ESB

#endif
