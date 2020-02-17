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
  SharedAllocator(Allocator *allocator);

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

  /** Initialize this memory pool.
   *
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error initialize();

  /** Destroy this allocator.  The allocator will return all of the memory
   *  it manages to its source allocator and return itself to a state where
   *  initialize could be called again.  All memory should be returned to
   *  the allocator before calling its destroy method.   Some implementations
   *  may refuse to shutdown if they have outstanding allocations.
   *
   *  @return ESB_SUCCESS if the allocator could destroy itself, another
   *      error code otherwise.  ESB_IN_USE will be returned if the
   *      allocator has handed out memory that has not been returned.
   *  @see initialize.
   */
  virtual Error destroy();

  /** Get the allocator's current initialization state.
   *
   *  @return ESB_SUCCESS if the allocator is initialized,
   *      ESB_NOT_INITIALIZED if the allocator has not been initialized,
   *      another error code if an error occurred determining the
   *      allocator's current state.
   *  @see initialize.
   */
  virtual Error isInitialized();

  /** Set another allocator to be used if this allocator cannot fulfill a
   *  allocate request.  This failover allocator will be initialized lazily.
   *  If a failover allocator is not set, an allocator will simply return
   *  NULL if it cannot fulfill an allocation request.  Destroying an
   *  allocator with registered failover allocators must also destroy those
   *  failover allocators if they have been initialized.  Failover allocators
   *  may be chained.  An allocator is responsible for determining whether
   *  a block of memory to be deallocated came from itself or from its
   *  failover allocator and take the appropriate action.  Allocators should
   *  never be used as failover allocators for themselves to avoid
   *  infinite recursion.
   *
   *  @param allocator The failover allocator.  Set to NULL to clear an
   *      already registered failover allocator.
   *  @return ESB_SUCCESS if successful, another error code otherwise.
   */
  virtual Error setFailoverAllocator(Allocator *allocator);

  /** Get the failover allocator used by this allocator.
   *
   *  @param allocator The failover allocator to get (pointer to a pointer).
   *      This pointer will be set to NULL if no failover allocator exists.
   *  @return ESB_SUCCESS if successful, another error code otherwise.  Note
   *      that if no failover allocator exists, ESB_SUCCESS will be returned
   *      and the allocator argument will be set to NULL.
   *  @see setFailoverAllocator
   */
  virtual Error getFailoverAllocator(Allocator **allocator);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  //  Disabled
  SharedAllocator(const SharedAllocator &);
  SharedAllocator &operator=(const SharedAllocator &);

  Allocator *_allocator;
  Mutex _mutex;
};

}  // namespace ESB

#endif
