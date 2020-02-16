/** @file ESFFixedAllocator.cpp
 *  @brief A ESFAllocator implementation good for fixed-length allocations.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_FIXED_ALLOCATOR_H
#include <ESFFixedAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

ESFFixedAllocator::ESFFixedAllocator(int blocks, int blockSize,
                                     ESFAllocator *source)
    : _availList(0),
      _pool(0),
      _sourceAllocator(source),
      _failoverAllocator(0),
      _blockSize(blockSize),
      _blocks(blocks) {}

ESFFixedAllocator::~ESFFixedAllocator() { destroy(); }

void *ESFFixedAllocator::allocate(ESFUWord size) {
  if (!_pool) {
    if (ESF_SUCCESS != initialize()) {
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

ESFError ESFFixedAllocator::deallocate(void *block) {
  if (!block) {
    return ESF_NULL_POINTER;
  }

  if (!_pool) {
    return ESF_INVALID_STATE;
  }

  char *elem = ((char *)block) - sizeof(AvailListElem);

  if (elem < _pool ||
      elem > (char *)_pool + (_blocks * (_blockSize + sizeof(AvailListElem)) -
                              _blockSize - sizeof(AvailListElem))) {
    if (_failoverAllocator) {
      return _failoverAllocator->deallocate(block);
    }

    return ESF_NOT_OWNER;
  }

  ((AvailListElem *)elem)->_next = _availList;
  _availList = (AvailListElem *)elem;

  return ESF_SUCCESS;
}

ESFUWord ESFFixedAllocator::getOverhead() { return ESF_UWORD_C(0); }

ESFError ESFFixedAllocator::initialize() {
  if (_pool || 0 >= _blocks || 0 >= _blockSize || !_sourceAllocator) {
    return ESF_INVALID_STATE;
  }

  _pool = _sourceAllocator->allocate(_blocks *
                                     (_blockSize + sizeof(AvailListElem)));

  if (0 == _pool) {
    return ESF_OUT_OF_MEMORY;
  }

  ESF_ASSERT(_pool);

  _availList = (AvailListElem *)_pool;

  char *elem = (char *)_pool;
  char *next = 0;

  for (int i = 0; i < _blocks - 1; ++i) {
    next = (elem + sizeof(AvailListElem) + _blockSize);

    ((AvailListElem *)elem)->_next = (AvailListElem *)next;

    elem = next;
  }

  ((AvailListElem *)elem)->_next = 0;

  return ESF_SUCCESS;
}

ESFError ESFFixedAllocator::destroy() {
  if (!_pool) {
    return ESF_INVALID_STATE;
  }

  int blocks = 0;

  for (AvailListElem *elem = _availList; elem; elem = elem->_next, ++blocks) {
  }

  if (blocks < _blocks) {
    return ESF_IN_USE;
  }

  if (_failoverAllocator) {
    ESFError error = _failoverAllocator->destroy();

    if (ESF_SUCCESS != error) {
      return error;
    }
  }

  _sourceAllocator->deallocate((void *)_pool);
  _pool = 0;

  return ESF_SUCCESS;
}

ESFError ESFFixedAllocator::isInitialized() {
  return _pool ? ESF_SUCCESS : ESF_NOT_INITIALIZED;
}

ESFError ESFFixedAllocator::setFailoverAllocator(ESFAllocator *allocator) {
  _failoverAllocator = allocator;

  return ESF_SUCCESS;
}

ESFError ESFFixedAllocator::getFailoverAllocator(ESFAllocator **allocator) {
  if (!allocator) {
    return ESF_NULL_POINTER;
  }

  *allocator = _failoverAllocator;

  return ESF_SUCCESS;
}

int ESFFixedAllocator::getBlockSize() { return _blockSize; }
