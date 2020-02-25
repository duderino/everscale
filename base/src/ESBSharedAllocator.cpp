#ifndef ESB_SHARED_ALLOCATOR_H
#include <ESBSharedAllocator.h>
#endif

namespace ESB {

SharedAllocator::SharedAllocator(Allocator *allocator)
    : _allocator(allocator), _mutex() {
  assert(_allocator);
}

SharedAllocator::~SharedAllocator() {}

void *SharedAllocator::allocate(UWord size) {
  if (ESB_SUCCESS != _mutex.writeAcquire()) {
    return 0;
  }

  void *block = _allocator->allocate(size);

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

  error = _allocator->deallocate(block);

  _mutex.writeRelease();

  return error;
}

Error SharedAllocator::initialize() {
  Error error = _mutex.writeAcquire();

  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator->initialize();

  _mutex.writeRelease();

  return error;
}

Error SharedAllocator::destroy() {
  Error error = _mutex.writeAcquire();

  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator->destroy();

  _mutex.writeRelease();

  return error;
}

UWord SharedAllocator::getOverhead() {
  /** @todo document that allocators should make sure this method is
   *  threadsafe
   */
  return _allocator->getOverhead();
}

Error SharedAllocator::isInitialized() {
  Error error = _mutex.readAcquire();

  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator->isInitialized();

  _mutex.readRelease();

  return error;
}

Error SharedAllocator::setFailoverAllocator(Allocator *allocator) {
  Error error = _mutex.writeAcquire();

  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator->setFailoverAllocator(allocator);

  _mutex.writeRelease();

  return error;
}

Error SharedAllocator::getFailoverAllocator(Allocator **allocator) {
  if (!allocator) {
    return ESB_NULL_POINTER;
  }

  Error error = _mutex.readAcquire();

  if (ESB_SUCCESS != error) {
    return error;
  }

  error = _allocator->getFailoverAllocator(allocator);

  _mutex.readRelease();

  return error;
}

}  // namespace ESB
