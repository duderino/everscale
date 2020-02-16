/** @file ESFEmbeddedList.cpp
 *  @brief A doubly linked list of ESFEmbeddedListElements
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).v
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

ESFEmbeddedList::ESFEmbeddedList() :
    _head(0), _tail(0) {
}

ESFEmbeddedList::~ESFEmbeddedList() {
}

void ESFEmbeddedList::clear(bool cleanup) {
    if (false == cleanup) {
        _head = 0;
        _tail = 0;

        return;
    }

    ESFCleanupHandler *cleanupHandler = 0;

    for (ESFEmbeddedListElement *element = removeFirst(); element; element = removeFirst()) {
        cleanupHandler = element->getCleanupHandler();

        if (cleanupHandler) {
            cleanupHandler->destroy(element);
        }
    }
}

int ESFEmbeddedList::length() const {
    int length = 0;

    for (ESFEmbeddedListElement *current = _head; current; current = current->getNext()) {
        ++length;
    }

    return length;
}

void ESFEmbeddedList::remove(ESFEmbeddedListElement *element) {
    ESF_ASSERT(element);
    ESF_ASSERT(_head);
    ESF_ASSERT(_tail);

    if (!element || 0 == _head || 0 == _tail) {
        return;
    }

    if (element->getPrevious()) {
        element->getPrevious()->setNext(element->getNext());
    } else {
        ESF_ASSERT(_head == element);

        _head = element->getNext();

        if (_head) {
            _head->setPrevious(0);
        }
    }

    if (element->getNext()) {
        element->getNext()->setPrevious(element->getPrevious());
    } else {
        ESF_ASSERT(_tail == element);

        _tail = element->getPrevious();

        if (_tail) {
            _tail->setNext(0);
        }
    }

    //element->setNext(0);
    //element->setPrevious(0);
}

void ESFEmbeddedList::addFirst(ESFEmbeddedListElement *element) {
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

void ESFEmbeddedList::addLast(ESFEmbeddedListElement *element) {
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

bool ESFEmbeddedList::validate() const {
    for (ESFEmbeddedListElement *current = _head; current; current = current->getNext()) {
        if (current->getNext()) {
            ESF_ASSERT(current == current->getNext()->getPrevious());

            if (current != current->getNext()->getPrevious()) {
                return false;
            }
        } else {
            ESF_ASSERT(current == _tail);

            if (current != _tail) {
                return false;
            }
        }

        if (current->getPrevious()) {
            ESF_ASSERT(current == current->getPrevious()->getNext());

            if (current != current->getPrevious()->getNext()) {
                return false;
            }
        } else {
            ESF_ASSERT(current == _head);

            if (current != _head) {
                return false;
            }
        }
    }

    return true;
}

