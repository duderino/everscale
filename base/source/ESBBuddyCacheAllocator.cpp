#ifndef ESB_BUDDY_CACHE_ALLOCATOR_H
#include <ESBBuddyCacheAllocator.h>
#endif

namespace ESB {

BuddyCacheAllocator::BuddyCacheAllocator(UInt32 size, Allocator &source, Allocator &failover)
    : BuddyAllocator(size, source), _cacheBytes(0), _failoverBytes(0), _failover(failover) {}
BuddyCacheAllocator::~BuddyCacheAllocator() {}

Error BuddyCacheAllocator::allocate(UWord size, void **block) {
  Error error = BuddyAllocator::allocate(size, block);
  if (ESB_SUCCESS == error) {
    _cacheBytes += size;
    return ESB_SUCCESS;
  }

  error = _failover.allocate(size, block);
  if (ESB_SUCCESS == error) {
    _failoverBytes += size;
  }
  return error;
}

Error BuddyCacheAllocator::deallocate(void *block) {
  Error error = BuddyAllocator::deallocate(block);
  return ESB_NOT_OWNER == error ? _failover.deallocate(block) : error;
}

bool BuddyCacheAllocator::reallocates() {
  assert(BuddyAllocator::reallocates());
  return _failover.reallocates();
}

Error BuddyCacheAllocator::reallocate(void *oldBlock, UWord newBlockSize, void **newBlock) {
  if (!_failover.reallocates()) {
    return ESB_OPERATION_NOT_SUPPORTED;
  }

  Error error = BuddyAllocator::reallocate(oldBlock, newBlockSize, newBlock);
  switch (error) {
    case ESB_SUCCESS:
      _cacheBytes += newBlockSize;
      return ESB_SUCCESS;
    case ESB_NOT_OWNER:
      error = _failover.reallocate(oldBlock, newBlockSize, newBlock);
      if (ESB_SUCCESS == error) {
        _failoverBytes += newBlockSize;
      }
      return error;
    case ESB_OUT_OF_MEMORY: {
      void *block = NULL;
      error = _failover.allocate(newBlockSize, &block);
      if (ESB_SUCCESS != error) {
        return error;
      }
      _failoverBytes += newBlockSize;
      UInt32 oldBlockSize = allocationSize(oldBlock);
      assert(0 < oldBlockSize);  // because otherwise case ESB_NOT_OWNER
      memcpy(block, oldBlock, MIN(oldBlockSize, newBlockSize));
      *newBlock = block;
      BuddyAllocator::deallocate(oldBlock);
      return ESB_SUCCESS;
    }
    default:
      return error;
  }
}

}  // namespace ESB
