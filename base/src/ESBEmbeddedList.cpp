#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ESB {

EmbeddedList::EmbeddedList() : _head(0), _tail(0) {}

EmbeddedList::~EmbeddedList() {}

void EmbeddedList::clear(bool cleanup) {
  if (false == cleanup) {
    _head = 0;
    _tail = 0;

    return;
  }

  CleanupHandler *cleanupHandler = 0;

  for (EmbeddedListElement *element = removeFirst(); element;
       element = removeFirst()) {
    cleanupHandler = element->getCleanupHandler();

    if (cleanupHandler) {
      cleanupHandler->destroy(element);
    }
  }
}

int EmbeddedList::length() const {
  int length = 0;

  for (EmbeddedListElement *current = _head; current;
       current = current->getNext()) {
    ++length;
  }

  return length;
}

void EmbeddedList::remove(EmbeddedListElement *element) {
  assert(element);
  assert(_head);
  assert(_tail);

  if (!element || 0 == _head || 0 == _tail) {
    return;
  }

#ifndef NDEBUG
  {
    bool foundElement = false;
    for (EmbeddedListElement *elem = _head; elem; elem = elem->getNext()) {
      if (elem == element) {
        foundElement = true;
      }
    }
    assert(foundElement);
  }
#endif

  if (element->getPrevious()) {
    element->getPrevious()->setNext(element->getNext());
  } else {
    assert(_head == element);

    _head = element->getNext();

    if (_head) {
      _head->setPrevious(0);
    }
  }

  if (element->getNext()) {
    element->getNext()->setPrevious(element->getPrevious());
  } else {
    assert(_tail == element);

    _tail = element->getPrevious();

    if (_tail) {
      _tail->setNext(0);
    }
  }

  element->setNext(0);
  element->setPrevious(0);
}

void EmbeddedList::addFirst(EmbeddedListElement *element) {
  assert(!element->getNext());
  assert(!element->getPrevious());
  if (0 == _head) {
    _head = element;
    _tail = element;
    element->setNext(0);
    element->setPrevious(0);

    return;
  }

  element->setNext(_head);
  element->setPrevious(0);
  _head->setPrevious(element);
  _head = element;
}

void EmbeddedList::addLast(EmbeddedListElement *element) {
  assert(!element->getNext());
  assert(!element->getPrevious());
  if (0 == _tail) {
    _head = element;
    _tail = element;
    element->setNext(0);
    element->setPrevious(0);

    return;
  }

  element->setNext(0);
  element->setPrevious(_tail);
  _tail->setNext(element);
  _tail = element;
}

bool EmbeddedList::validate() const {
  for (EmbeddedListElement *current = _head; current;
       current = current->getNext()) {
    if (current->getNext()) {
      assert(current == current->getNext()->getPrevious());

      if (current != current->getNext()->getPrevious()) {
        return false;
      }
    } else {
      assert(current == _tail);

      if (current != _tail) {
        return false;
      }
    }

    if (current->getPrevious()) {
      assert(current == current->getPrevious()->getNext());

      if (current != current->getPrevious()->getNext()) {
        return false;
      }
    } else {
      assert(current == _head);

      if (current != _head) {
        return false;
      }
    }
  }

  return true;
}

}  // namespace ESB
