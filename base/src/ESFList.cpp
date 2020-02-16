/** @file ESFList.cpp
 *  @brief A doubly-linked list that stores its values as void pointers
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

#ifndef ESF_LIST_H
#include <ESFList.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

ESFListIterator::ESFListIterator() : _node(0) {}

ESFListIterator::ESFListIterator(ESFListNode *node) : _node(node) {}

ESFListIterator::ESFListIterator(const ESFListIterator &iterator)
    : _node(iterator._node) {}

ESFListIterator::~ESFListIterator() {}

ESFList::ESFList(ESFAllocator *allocator, ESFLockable *lockable)
    : _size(0),
      _head(0),
      _tail(0),
      _allocator(allocator),
      _lockable(lockable) {}

ESFList::~ESFList() { clear(); }

ESFError ESFList::pushFront(void *element) {
  if (!element) {
    return ESF_NULL_POINTER;
  }

  if (!_allocator || !_lockable) {
    return ESF_INVALID_STATE;
  }

  if (ESF_UINT32_MAX == _size) {
    return ESF_OVERFLOW;
  }

  ESFListNode *node = (ESFListNode *)_allocator->allocate(sizeof(ESFListNode));

  if (0 == node) {
    return ESF_OUT_OF_MEMORY;
  }

  ESF_ASSERT(node);

  node->_value = element;
  node->_prev = 0;
  node->_next = 0;

  if (!_head) {
    ESF_ASSERT(!_tail);

    _head = node;
    _tail = node;

    ++_size;

    return ESF_SUCCESS;
  }

  node->_next = _head;
  _head->_prev = node;
  _head = node;

  ++_size;

  return ESF_SUCCESS;
}

ESFError ESFList::pushBack(void *element) {
  if (!element) {
    return ESF_NULL_POINTER;
  }

  if (!_allocator || !_lockable) {
    return ESF_INVALID_STATE;
  }

  if (ESF_UINT32_MAX == _size) {
    return ESF_OVERFLOW;
  }

  ESFListNode *node = (ESFListNode *)_allocator->allocate(sizeof(ESFListNode));

  if (0 == node) {
    return ESF_OUT_OF_MEMORY;
  }

  ESF_ASSERT(node);

  node->_value = element;
  node->_prev = 0;
  node->_next = 0;

  if (!_tail) {
    ESF_ASSERT(!_head);

    _head = node;
    _tail = node;

    ++_size;

    return ESF_SUCCESS;
  }

  node->_prev = _tail;
  _tail->_next = node;
  _tail = node;

  ++_size;

  return ESF_SUCCESS;
}

void *ESFList::getFront() {
  if (!_head) {
    return 0;
  }

  return _head->_value;
}

ESFError ESFList::popFront() {
  if (!_allocator || !_lockable) {
    return ESF_INVALID_STATE;
  }

  if (!_head) {
    return ESF_SUCCESS;
  }

  ESFListNode *node = _head;

  _head = _head->_next;

  if (_head) {
    _head->_prev = 0;
  } else {
    _tail = 0;
  }

  ESFError error = _allocator->deallocate((void *)node);

  if (ESF_SUCCESS == error) {
    --_size;

    return ESF_SUCCESS;
  }

  // failed to delete the node, restore the node's position in the list

  ESF_ASSERT(node);

  node->_next = _head;

  if (_head) {
    _head->_prev = node;
  }

  _head = node;

  return error;
}

void *ESFList::getBack() {
  if (!_tail) {
    return 0;
  }

  return _tail->_value;
}

ESFError ESFList::popBack() {
  if (!_allocator || !_lockable) {
    return ESF_INVALID_STATE;
  }

  if (!_tail) {
    return ESF_SUCCESS;
  }

  ESFListNode *node = _tail;

  _tail = _tail->_prev;

  if (_tail) {
    _tail->_next = 0;
  } else {
    _head = 0;
  }

  ESFError error = _allocator->deallocate((void *)node);

  if (ESF_SUCCESS == error) {
    --_size;

    return ESF_SUCCESS;
  }

  // failed to delete the node, restore the node's position in the list

  ESF_ASSERT(node);

  node->_prev = _tail;

  if (_tail) {
    _tail->_next = node;
  }

  _tail = node;

  return error;
}

ESFError ESFList::clear() {
  if (!_allocator || !_lockable) {
    return ESF_INVALID_STATE;
  }

  ESFError error = ESF_SUCCESS;
  ESFListNode *current = _head;
  ESFListNode *next = 0;

  while (current) {
    next = current->_next;

    error = _allocator->deallocate((void *)current);

    if (ESF_SUCCESS != error) {
      // failed to delete the node, restore the list to a stable state

      ESF_ASSERT(current);

      current->_prev = 0;
      _head = current;

      return error;
    }

    --_size;

    current = next;
  }

  _head = 0;
  _tail = 0;

  ESF_ASSERT(0 == _size);

  return ESF_SUCCESS;
}

ESFListIterator ESFList::getFrontIterator() {
  ESFListIterator iterator(_head);

  return iterator;
}

ESFListIterator ESFList::getBackIterator() {
  ESFListIterator iterator(_tail);

  return iterator;
}

ESFError ESFList::erase(ESFListIterator *iterator) {
  if (!iterator || iterator->isNull()) {
    return ESF_INVALID_ITERATOR;
  }

  if (!_allocator || !_lockable) {
    return ESF_INVALID_STATE;
  }

  ESFError error = deleteNode(iterator->_node);

  if (ESF_SUCCESS == error) {
    --_size;

    iterator->_node = 0;
  }

  return error;
}

ESFError ESFList::deleteNode(ESFListNode *node) {
  if (!node) {
    return ESF_NULL_POINTER;
  }

  if (node->_prev) {
    node->_prev->_next = node->_next;

    if (!node->_next) _tail = node->_prev;
  } else {
    _head = node->_next;

    if (node->_next) node->_next->_prev = 0;
  }

  if (node->_next) {
    node->_next->_prev = node->_prev;

    if (!node->_prev) _head = node->_next;
  } else {
    _tail = node->_prev;

    if (node->_prev) node->_prev->_next = 0;
  }

  ESFError error = _allocator->deallocate((void *)node);

  if (ESF_SUCCESS == error) {
    return ESF_SUCCESS;
  }

  // failed to delete the node, restore the node in the list.

  ESF_ASSERT(node);

  if (node->_prev) {
    node->_prev->_next = node;
  } else {
    _head = node;
  }

  if (node->_next) {
    node->_next->_prev = node;
  } else {
    _tail = node;
  }

  return error;
}

ESFUInt32 ESFList::getSize() const { return _size; }

bool ESFList::isEmpty() const { return !_head; }

ESFSize ESFList::GetAllocationSize() { return sizeof(ESFListNode); }

ESFError ESFList::writeAcquire() {
  if (!_lockable) return ESF_INVALID_STATE;

  return _lockable->writeAcquire();
}

ESFError ESFList::readAcquire() {
  if (!_lockable) return ESF_INVALID_STATE;

  return _lockable->readAcquire();
}

ESFError ESFList::writeAttempt() {
  if (!_lockable) return ESF_INVALID_STATE;

  return _lockable->writeAttempt();
}

ESFError ESFList::readAttempt() {
  if (!_lockable) return ESF_INVALID_STATE;

  return _lockable->readAttempt();
}

ESFError ESFList::writeRelease() {
  if (!_lockable) return ESF_INVALID_STATE;

  return _lockable->writeRelease();
}

ESFError ESFList::readRelease() {
  if (!_lockable) return ESF_INVALID_STATE;

  return _lockable->readRelease();
}
