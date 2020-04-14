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

  for (EmbeddedListElement *element = removeFirst(); element;
       element = removeFirst()) {
    CleanupHandler *cleanupHandler = element->cleanupHandler();

    if (cleanupHandler) {
      cleanupHandler->destroy(element);
    }
  }
}

int EmbeddedList::size() const {
  int length = 0;

  for (EmbeddedListElement *current = _head; current;
       current = current->next()) {
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
    for (EmbeddedListElement *elem = _head; elem; elem = elem->next()) {
      if (elem == element) {
        foundElement = true;
      }
    }
    assert(foundElement);
  }
#endif

  if (element->previous()) {
    element->previous()->setNext(element->next());
  } else {
    assert(_head == element);

    _head = element->next();

    if (_head) {
      _head->setPrevious(0);
    }
  }

  if (element->next()) {
    element->next()->setPrevious(element->previous());
  } else {
    assert(_tail == element);

    _tail = element->previous();

    if (_tail) {
      _tail->setNext(0);
    }
  }

  element->setNext(0);
  element->setPrevious(0);
}

void EmbeddedList::addFirst(EmbeddedListElement *element) {
  assert(!element->next());
  assert(!element->previous());
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
  assert(!element->next());
  assert(!element->previous());
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

EmbeddedListElement *EmbeddedList::index(int idx) {
  int i = 0;
  for (EmbeddedListElement *elem = first(); elem; elem = elem->next(), ++i) {
    if (i == idx) {
      return elem;
    }
  }

  return NULL;
}

bool EmbeddedList::validate() const {
  for (EmbeddedListElement *current = _head; current;
       current = current->next()) {
    if (current->next()) {
      assert(current == current->next()->previous());

      if (current != current->next()->previous()) {
        return false;
      }
    } else {
      assert(current == _tail);

      if (current != _tail) {
        return false;
      }
    }

    if (current->previous()) {
      assert(current == current->previous()->next());

      if (current != current->previous()->next()) {
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
