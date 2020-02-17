#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

namespace ESB {

FixedAllocator::FixedAllocator(int blocks, int blockSize, Allocator *source)
    : _availList(0),
      _pool(0),
      _sourceAllocator(source),
      _failoverAllocator(0),
      _blockSize(blockSize),
      _blocks(blocks) {}

FixedAllocator::~FixedAllocator() { destroy(); }

void *FixedAllocator::allocate(UWord size) {
  if (!_pool) {
    if (ESB_SUCCESS != initialize()) {
      return 0;
    }
  }

  if (!_availList || size > _blockSize) {
    if (_failoverAllocator) {
      return _failoverAllocator->allocate(size);
    }

    return 0;
  }

  char *elem = ((char *)_availList) + sizeof(AvailListElem);

  _availList = _availList->_next;

  return elem;
}

Error FixedAllocator::deallocate(void *block) {
  if (!block) {
    return ESB_NULL_POINTER;
  }

  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  char *elem = ((char *)block) - sizeof(AvailListElem);

  if (elem < _pool ||
      elem > (char *)_pool + (_blocks * (_blockSize + sizeof(AvailListElem)) -
                              _blockSize - sizeof(AvailListElem))) {
    if (_failoverAllocator) {
      return _failoverAllocator->deallocate(block);
    }

    return ESB_NOT_OWNER;
  }

  ((AvailListElem *)elem)->_next = _availList;
  _availList = (AvailListElem *)elem;

  return ESB_SUCCESS;
}

UWord FixedAllocator::getOverhead() { return ESB_UWORD_C(0); }

Error FixedAllocator::initialize() {
  if (_pool || 0 >= _blocks || 0 >= _blockSize || !_sourceAllocator) {
    return ESB_INVALID_STATE;
  }

  _pool = _sourceAllocator->allocate(_blocks *
                                     (_blockSize + sizeof(AvailListElem)));

  if (0 == _pool) {
    return ESB_OUT_OF_MEMORY;
  }

  assert(_pool);

  _availList = (AvailListElem *)_pool;

  char *elem = (char *)_pool;
  char *next = 0;

  for (int i = 0; i < _blocks - 1; ++i) {
    next = (elem + sizeof(AvailListElem) + _blockSize);

    ((AvailListElem *)elem)->_next = (AvailListElem *)next;

    elem = next;
  }

  ((AvailListElem *)elem)->_next = 0;

  return ESB_SUCCESS;
}

Error FixedAllocator::destroy() {
  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  int blocks = 0;

  for (AvailListElem *elem = _availList; elem; elem = elem->_next, ++blocks) {
  }

  if (blocks < _blocks) {
    return ESB_IN_USE;
  }

  if (_failoverAllocator) {
    Error error = _failoverAllocator->destroy();

    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  _sourceAllocator->deallocate((void *)_pool);
  _pool = 0;

  return ESB_SUCCESS;
}

Error FixedAllocator::isInitialized() {
  return _pool ? ESB_SUCCESS : ESB_NOT_INITIALIZED;
}

Error FixedAllocator::setFailoverAllocator(Allocator *allocator) {
  _failoverAllocator = allocator;

  return ESB_SUCCESS;
}

Error FixedAllocator::getFailoverAllocator(Allocator **allocator) {
  if (!allocator) {
    return ESB_NULL_POINTER;
  }

  *allocator = _failoverAllocator;

  return ESB_SUCCESS;
}

int FixedAllocator::getBlockSize() { return _blockSize; }

}  // namespace ESB
