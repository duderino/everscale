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
 *    system's global memory allocation routines (e.g., malloc and free).
 *
 *  @ingroup allocator
 */
class SystemAllocator : public Allocator {
 public:
  static SystemAllocator *GetInstance();

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

  /** Allocate a word-aligned memory block of at least size bytes.
   *
   *  @param block The block to allocate (pointer to a pointer)
   *  @param size The minimum number of bytes to allocate.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error allocate(void **block, UWord size);

  /** Deallocate a memory block allocated by this allocator or by its
   *  failover allocators.
   *
   *  @param block The block to deallocate.
   *  @return ESB_SUCCESS if the block was successfully deallocated, another
   *      error code otherwise.  ESB_NOT_OWNER will be returned if the
   *      block was not allocated by this allocator.
   */
  virtual Error deallocate(void *block);

  /** Get the overhead in bytes of any additional control structures
   *  attached to an allocated block.  This can be used to optimize
   *  allocation requests for some allocators.  Power of two allocators,
   *  for example, will always round the size requested + the overhead up
   *  to the next power of two.  If the caller always requests powers of
   *  two, then the allocator can waste a lot of memory.  If, however, the
   *  caller requests a power of two minus the allocator overhead, then
   *  the allocator will return the optimal amount of memory.
   *
   *  @return the allocator's overhead in bytes for each allocation.
   */
  virtual UWord getOverhead();

  /** Initialize this memory pool.  This operation does not need to be
   *  called on the SystemAllocator, the SystemAllocator is always
   *  in the initialized state.
   *
   *  @return ESB_SUCCESS.
   */
  virtual Error initialize();

  /** Destroy this allocator.  This operation is not supported, the
   *  SystemAllocator is always in the initialized state.
   *
   *  @return ESB_OPERATION_NOT_SUPPORTED.
   *  @see initialize.
   */
  virtual Error destroy();

  /** Get the allocators current initialization state.  The
   *  SystemAllocator is always in the initialized state.
   *
   *  @return ESB_SUCCESS.
   *  @see initialize.
   */
  virtual Error isInitialized();

  /** Set another allocator to be used if this allocator cannot fulfill a
   *  allocate request.  The SystemAllocator does not support failover
   *  allocators (it cannot distinguish between blocks it allocates and
   *  blocks its failover allocator allocates).
   *
   *  @param allocator The failover allocator.  Set to NULL to clear an
   *      already registered failover allocator.
   *  @return ESB_OPERATION_NOT_SUPPORTED
   */
  virtual Error setFailoverAllocator(Allocator *allocator);

  /** Get the failover allocator used by this allocator.  The
   *  SystemAllocator does not support failover allocators (it cannot
   *  distinguish between blocks it allocates and blocks its failover
   *  allocator allocates).
   *
   *  @param allocator The failover allocator to get (pointer to a pointer).
   *  @return ESB_OPERATION_NOT_SUPPORTED.
   */
  virtual Error getFailoverAllocator(Allocator **allocator);

 private:
  //  Disabled
  SystemAllocator();
  SystemAllocator(const SystemAllocator &);
  SystemAllocator &operator=(const SystemAllocator &);

  static SystemAllocator _Allocator;
};

}  // namespace ESB

#endif
