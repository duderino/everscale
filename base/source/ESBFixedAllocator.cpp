#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

FixedAllocator::FixedAllocator(UInt32 blocks, UInt32 blockSize,
                               Allocator &source)
    : _availList(NULL),
      _pool(NULL),
      _sourceAllocator(source),
      _blockSize(blockSize),
      _blocks(blocks),
      _cleanupHandler(*this) {}

FixedAllocator::~FixedAllocator() { destroy(); }

void *FixedAllocator::allocate(UWord size) {
#ifdef ESB_NO_ALLOC
  return SystemAllocator::Instance().allocate(size);
#else

  if (!_pool) {
    if (ESB_SUCCESS != initialize()) {
      return NULL;
    }
  }

  if (!_availList || size > _blockSize) {
    return NULL;
  }

  char *elem = ((char *)_availList) + sizeof(AvailListElem);

  _availList = _availList->_next;

  return elem;
#endif
}

Error FixedAllocator::deallocate(void *block) {
#ifdef ESB_NO_ALLOC
  return SystemAllocator::Instance().deallocate(block);
#else
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
    return ESB_NOT_OWNER;
  }

  ((AvailListElem *)elem)->_next = _availList;
  _availList = (AvailListElem *)elem;

  return ESB_SUCCESS;
#endif
}

Error FixedAllocator::initialize() {
  if (_pool || 0 >= _blocks || 0 >= _blockSize) {
    return ESB_INVALID_STATE;
  }

  _pool =
      _sourceAllocator.allocate(_blocks * (_blockSize + sizeof(AvailListElem)));

  if (NULL == _pool) {
    return ESB_OUT_OF_MEMORY;
  }

  assert(_pool);

  _availList = (AvailListElem *)_pool;

  char *elem = (char *)_pool;
  char *next = NULL;

  for (UInt32 i = 0; i < _blocks - 1; ++i) {
    next = (elem + sizeof(AvailListElem) + _blockSize);

    ((AvailListElem *)elem)->_next = (AvailListElem *)next;

    elem = next;
  }

  ((AvailListElem *)elem)->_next = NULL;

  return ESB_SUCCESS;
}

Error FixedAllocator::destroy() {
  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  UInt32 blocks = 0U;

  for (AvailListElem *elem = _availList; elem; elem = elem->_next, ++blocks) {
  }

  if (blocks < _blocks) {
    return ESB_IN_USE;
  }

  _sourceAllocator.deallocate((void *)_pool);
  _pool = NULL;

  return ESB_SUCCESS;
}

CleanupHandler &FixedAllocator::cleanupHandler() { return _cleanupHandler; }

}  // namespace ESB
