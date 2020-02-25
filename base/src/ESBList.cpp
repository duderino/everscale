#ifndef ESB_LIST_H
#include <ESBList.h>
#endif

namespace ESB {

ListIterator::ListIterator() : _node(0) {}

ListIterator::ListIterator(ListNode *node) : _node(node) {}

ListIterator::ListIterator(const ListIterator &iterator)
    : _node(iterator._node) {}

ListIterator::~ListIterator() {}

List::List(Allocator *allocator, Lockable *lockable)
    : _size(0),
      _head(0),
      _tail(0),
      _allocator(allocator),
      _lockable(lockable) {}

List::~List() { clear(); }

Error List::pushFront(void *element) {
  if (!element) {
    return ESB_NULL_POINTER;
  }

  if (!_allocator || !_lockable) {
    return ESB_INVALID_STATE;
  }

  if (ESB_UINT32_MAX == _size) {
    return ESB_OVERFLOW;
  }

  ListNode *node = (ListNode *)_allocator->allocate(sizeof(ListNode));

  if (0 == node) {
    return ESB_OUT_OF_MEMORY;
  }

  assert(node);

  node->_value = element;
  node->_prev = 0;
  node->_next = 0;

  if (!_head) {
    assert(!_tail);

    _head = node;
    _tail = node;

    ++_size;

    return ESB_SUCCESS;
  }

  node->_next = _head;
  _head->_prev = node;
  _head = node;

  ++_size;

  return ESB_SUCCESS;
}

Error List::pushBack(void *element) {
  if (!element) {
    return ESB_NULL_POINTER;
  }

  if (!_allocator || !_lockable) {
    return ESB_INVALID_STATE;
  }

  if (ESB_UINT32_MAX == _size) {
    return ESB_OVERFLOW;
  }

  ListNode *node = (ListNode *)_allocator->allocate(sizeof(ListNode));

  if (0 == node) {
    return ESB_OUT_OF_MEMORY;
  }

  assert(node);

  node->_value = element;
  node->_prev = 0;
  node->_next = 0;

  if (!_tail) {
    assert(!_head);

    _head = node;
    _tail = node;

    ++_size;

    return ESB_SUCCESS;
  }

  node->_prev = _tail;
  _tail->_next = node;
  _tail = node;

  ++_size;

  return ESB_SUCCESS;
}

void *List::getFront() {
  if (!_head) {
    return 0;
  }

  return _head->_value;
}

Error List::popFront() {
  if (!_allocator || !_lockable) {
    return ESB_INVALID_STATE;
  }

  if (!_head) {
    return ESB_SUCCESS;
  }

  ListNode *node = _head;

  _head = _head->_next;

  if (_head) {
    _head->_prev = 0;
  } else {
    _tail = 0;
  }

  Error error = _allocator->deallocate((void *)node);

  if (ESB_SUCCESS == error) {
    --_size;

    return ESB_SUCCESS;
  }

  // failed to delete the node, restore the node's position in the list

  assert(node);

  node->_next = _head;

  if (_head) {
    _head->_prev = node;
  }

  _head = node;

  return error;
}

void *List::getBack() {
  if (!_tail) {
    return 0;
  }

  return _tail->_value;
}

Error List::popBack() {
  if (!_allocator || !_lockable) {
    return ESB_INVALID_STATE;
  }

  if (!_tail) {
    return ESB_SUCCESS;
  }

  ListNode *node = _tail;

  _tail = _tail->_prev;

  if (_tail) {
    _tail->_next = 0;
  } else {
    _head = 0;
  }

  Error error = _allocator->deallocate((void *)node);

  if (ESB_SUCCESS == error) {
    --_size;

    return ESB_SUCCESS;
  }

  // failed to delete the node, restore the node's position in the list

  assert(node);

  node->_prev = _tail;

  if (_tail) {
    _tail->_next = node;
  }

  _tail = node;

  return error;
}

Error List::clear() {
  if (!_allocator || !_lockable) {
    return ESB_INVALID_STATE;
  }

  Error error = ESB_SUCCESS;
  ListNode *current = _head;
  ListNode *next = 0;

  while (current) {
    next = current->_next;

    error = _allocator->deallocate((void *)current);

    if (ESB_SUCCESS != error) {
      // failed to delete the node, restore the list to a stable state

      assert(current);

      current->_prev = 0;
      _head = current;

      return error;
    }

    --_size;

    current = next;
  }

  _head = 0;
  _tail = 0;

  assert(0 == _size);

  return ESB_SUCCESS;
}

ListIterator List::getFrontIterator() {
  ListIterator iterator(_head);

  return iterator;
}

ListIterator List::getBackIterator() {
  ListIterator iterator(_tail);

  return iterator;
}

Error List::erase(ListIterator *iterator) {
  if (!iterator || iterator->isNull()) {
    return ESB_INVALID_ITERATOR;
  }

  if (!_allocator || !_lockable) {
    return ESB_INVALID_STATE;
  }

  Error error = deleteNode(iterator->_node);

  if (ESB_SUCCESS == error) {
    --_size;

    iterator->_node = 0;
  }

  return error;
}

Error List::deleteNode(ListNode *node) {
  if (!node) {
    return ESB_NULL_POINTER;
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

  Error error = _allocator->deallocate((void *)node);

  if (ESB_SUCCESS == error) {
    return ESB_SUCCESS;
  }

  // failed to delete the node, restore the node in the list.

  assert(node);

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

UInt32 List::getSize() const { return _size; }

bool List::isEmpty() const { return !_head; }

Size List::GetAllocationSize() { return sizeof(ListNode); }

Error List::writeAcquire() {
  if (!_lockable) return ESB_INVALID_STATE;

  return _lockable->writeAcquire();
}

Error List::readAcquire() {
  if (!_lockable) return ESB_INVALID_STATE;

  return _lockable->readAcquire();
}

Error List::writeAttempt() {
  if (!_lockable) return ESB_INVALID_STATE;

  return _lockable->writeAttempt();
}

Error List::readAttempt() {
  if (!_lockable) return ESB_INVALID_STATE;

  return _lockable->readAttempt();
}

Error List::writeRelease() {
  if (!_lockable) return ESB_INVALID_STATE;

  return _lockable->writeRelease();
}

Error List::readRelease() {
  if (!_lockable) return ESB_INVALID_STATE;

  return _lockable->readRelease();
}

}  // namespace ESB
