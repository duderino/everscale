#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

namespace ESB {

SharedAllocator::SharedAllocator(Allocator &allocator)
    : _allocator(allocator), _mutex(), _sharedCleanupHandler(*this) {}

SharedAllocator::~SharedAllocator() {}

Error SharedAllocator::allocate(UWord size, void **block) {
  Error error = _mutex.writeAcquire();
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator.allocate(size, block);
  _mutex.writeRelease();
  return error;
}

Error SharedAllocator::deallocate(void *block) {
  if (!block) {
    return ESB_NULL_POINTER;
  }

  Error error = _mutex.writeAcquire();
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator.deallocate(block);
  _mutex.writeRelease();
  return error;
}

CleanupHandler &SharedAllocator::cleanupHandler() { return _sharedCleanupHandler; }

bool SharedAllocator::reallocates() { return _allocator.reallocates(); }

Error SharedAllocator::reallocate(void *oldBlock, UWord size, void **newBlock) {
  Error error = _mutex.writeAcquire();
  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator.reallocate(oldBlock, size, newBlock);
  _mutex.writeRelease();
  return error;
}

}  // namespace ESB
