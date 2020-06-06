#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

namespace ESB {

SharedAllocator::SharedAllocator(Allocator &allocator)
    : _allocator(allocator), _mutex(), _sharedCleanupHandler(*this) {}

SharedAllocator::~SharedAllocator() {}

void *SharedAllocator::allocate(UWord size) {
  if (ESB_SUCCESS != _mutex.writeAcquire()) {
    return 0;
  }

  void *block = _allocator.allocate(size);

  _mutex.writeRelease();

  return block;
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

}  // namespace ESB
