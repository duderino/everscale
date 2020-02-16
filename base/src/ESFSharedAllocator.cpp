/** @file ESFSharedAllocator.cpp
 *  @brief A ESFAllocator decorator that can synchronize access to any allocator
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_SHARED_ALLOCATOR_H
#include <ESFSharedAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

ESFSharedAllocator::ESFSharedAllocator(ESFAllocator *allocator)
    : _allocator(allocator), _mutex() {
  ESF_ASSERT(_allocator);
}

ESFSharedAllocator::~ESFSharedAllocator() {}

void *ESFSharedAllocator::allocate(ESFUWord size) {
  if (ESF_SUCCESS != _mutex.writeAcquire()) {
    return 0;
  }

  void *block = _allocator->allocate(size);

  _mutex.writeRelease();

  return block;
}

ESFError ESFSharedAllocator::deallocate(void *block) {
  if (!block) {
    return ESF_NULL_POINTER;
  }

  ESFError error = _mutex.writeAcquire();

  if (ESF_SUCCESS != error) {
    return error;
  }

  error = _allocator->deallocate(block);

  _mutex.writeRelease();

  return error;
}

ESFError ESFSharedAllocator::initialize() {
  ESFError error = _mutex.writeAcquire();

  if (ESF_SUCCESS != error) {
    return error;
  }

  error = _allocator->initialize();

  _mutex.writeRelease();

  return error;
}

ESFError ESFSharedAllocator::destroy() {
  ESFError error = _mutex.writeAcquire();

  if (ESF_SUCCESS != error) {
    return error;
  }

  error = _allocator->destroy();

  _mutex.writeRelease();

  return error;
}

ESFUWord ESFSharedAllocator::getOverhead() {
  /** @todo document that allocators should make sure this method is
   *  threadsafe
   */
  return _allocator->getOverhead();
}

ESFError ESFSharedAllocator::isInitialized() {
  ESFError error = _mutex.readAcquire();

  if (ESF_SUCCESS != error) {
    return error;
  }

  error = _allocator->isInitialized();

  _mutex.readRelease();

  return error;
}

ESFError ESFSharedAllocator::setFailoverAllocator(ESFAllocator *allocator) {
  ESFError error = _mutex.writeAcquire();

  if (ESF_SUCCESS != error) {
    return error;
  }

  error = _allocator->setFailoverAllocator(allocator);

  _mutex.writeRelease();

  return error;
}

ESFError ESFSharedAllocator::getFailoverAllocator(ESFAllocator **allocator) {
  if (!allocator) {
    return ESF_NULL_POINTER;
  }

  ESFError error = _mutex.readAcquire();

  if (ESF_SUCCESS != error) {
    return error;
  }

  error = _allocator->getFailoverAllocator(allocator);

  _mutex.readRelease();

  return error;
}
