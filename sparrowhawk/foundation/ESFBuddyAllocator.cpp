/** @file ESFBuddyAllocator.cpp
 *  @brief A ESFAllocator implementation good for variable-length allocations
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_BUDDY_ALLOCATOR_H
#include <ESFBuddyAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

ESFBuddyAllocator::KVal ESFBuddyAllocator::GetKVal(ESFUWord requestedSize) {
    ESFUWord adjustedSize = requestedSize + sizeof(AvailListElem);
    KVal kVal = 1;

    while (adjustedSize > (ESF_UWORD_C(1) << kVal)) {
        ++kVal;
    }

    return kVal;
}

ESFBuddyAllocator::AvailListElem *ESFBuddyAllocator::popAvailList(ESFBuddyAllocator::KVal kVal) {
    ESF_ASSERT( kVal < ESF_AVAIL_LIST_LENGTH );

    AvailListElem *elem = _availList[kVal];

    if (!elem)
        return 0;

    AvailListElem *next = elem->_linkF;

    ESF_ASSERT( 0 == elem->_linkB );
    ESF_ASSERT( kVal == elem->_kVal );
    ESF_ASSERT( 1 == elem->_tag );

    elem->_linkB = 0;
    elem->_linkF = 0;
    elem->_tag = 0;

    if (next) {
        next->_linkB = 0;
        _availList[kVal] = next;
    } else {
        _availList[kVal] = 0;
    }

    return elem;
}

void ESFBuddyAllocator::removeFromAvailList(ESFBuddyAllocator::AvailListElem *elem) {
    ESF_ASSERT( elem );
    ESF_ASSERT( elem->_kVal < ESF_AVAIL_LIST_LENGTH );

    if (elem->_linkB) {
        elem->_linkB->_linkF = elem->_linkF;
    }

    if (elem->_linkF) {
        elem->_linkF->_linkB = elem->_linkB;
    }

    if (!elem->_linkB) {
        _availList[elem->_kVal] = elem->_linkF;
    }

    elem->_linkB = 0;
    elem->_linkF = 0;
    elem->_tag = 0;
}

void ESFBuddyAllocator::pushAvailList(AvailListElem *elem) {
    ESF_ASSERT( elem );
    ESF_ASSERT( elem->_kVal < ESF_AVAIL_LIST_LENGTH );

    AvailListElem *next = _availList[elem->_kVal];

    elem->_linkB = 0;
    elem->_linkF = next;
    elem->_tag = 1;

    if (next) {
        next->_linkB = elem;
    }

    _availList[elem->_kVal] = elem;
}

ESFBuddyAllocator::ESFBuddyAllocator(int size, ESFAllocator *source) :
    _failoverAllocator(0), _sourceAllocator(source), _pool(0), _poolKVal(0) {
    if (1 > size || ESF_AVAIL_LIST_LENGTH < size)
        return;

    if (!_sourceAllocator)
        return;

    for (int i = 0; i < ESF_AVAIL_LIST_LENGTH; ++i) {
        _availList[i] = 0;
    }

    _poolKVal = size;
    _sourceAllocator = source;
}

ESFBuddyAllocator::~ESFBuddyAllocator() {
    destroy();
}

void *
ESFBuddyAllocator::allocate(ESFUWord size) {
    if (0 == size) {
        return 0;
    }

    if (!_pool) {
        if (ESF_SUCCESS != initialize()) {
            return 0;
        }
    }

    //
    //    Find an element large enough to fulfill the request.  If found, remove
    //    it from the available list.  If not found, try the failover allocator.
    //
    //    Knuth R1 and R2
    //

    KVal minimumKVal = GetKVal(size);
    KVal actualKVal = minimumKVal;
    char *elem = 0;

    for (; actualKVal < ESF_AVAIL_LIST_LENGTH; ++actualKVal) {
        if (0 != _availList[actualKVal]) {
            elem = (char *) popAvailList(actualKVal);
            break;
        }
    }

    if (!elem) {
        if (!_failoverAllocator)
            return 0;

        return _failoverAllocator->allocate(size);
    }

    //
    //    Keep splitting the elem until we are left with a minimally sized elem.
    //
    //    Knuth R3 and R4
    //

    ESF_ASSERT( actualKVal >= minimumKVal );

    AvailListElem *right = (AvailListElem *) elem;
    AvailListElem *left = 0;

    while (minimumKVal < actualKVal) {
        --actualKVal;

        left = (AvailListElem *) (elem + (ESF_UWORD_C(1) << actualKVal));

        right->_kVal = actualKVal;

        left->_linkB = 0;
        left->_linkF = 0;
        left->_kVal = actualKVal;
        left->_tag = 0;

        pushAvailList(left);
    }

    ESF_ASSERT( right );

    return (void *) (((char *) right) + sizeof(AvailListElem));
}

ESFError ESFBuddyAllocator::allocate(void **block, ESFUWord size) {
    if (!block) {
        return ESF_NULL_POINTER;
    }

    if (0 == size) {
        return ESF_INVALID_ARGUMENT;
    }

    if (!_pool) {
        ESFError error = initialize();

        if (ESF_SUCCESS != error) {
            return error;
        }
    }

    void *tmp = allocate(size);

    if (!tmp) {
        return ESF_OUT_OF_MEMORY;
    }

    *block = tmp;

    return ESF_SUCCESS;
}

ESFError ESFBuddyAllocator::deallocate(void *block) {
    if (!block) {
        return ESF_NULL_POINTER;
    }

    if (!_pool) {
        return ESF_INVALID_STATE;
    }

    char *trueAddress = ((char *) block) - sizeof(AvailListElem);

    if (trueAddress < _pool || trueAddress >= _pool + (ESF_UWORD_C(1) << _poolKVal)) {
        if (_failoverAllocator) {
            return _failoverAllocator->deallocate(block);
        }

        return ESF_NOT_OWNER;
    }

    //
    //    Find this block's buddy and check its availability.
    //
    //    Knuth S1
    //

    AvailListElem *elem = (AvailListElem *) trueAddress;
    AvailListElem *buddy = 0;

    ESF_ASSERT( 0 == elem->_linkB );
    ESF_ASSERT( 0 == elem->_linkF );
    ESF_ASSERT( 0 == elem->_tag );

    //
    //  For as long as we can, start coalescing buddies.  Two buddies can be
    //  coalesced iff both are available and both are the same size (kVal).
    //  If they have different kVals, one of the buddies has been broken up
    //  into smaller nodes and cannot be coalesced until it is whole again.
    //
    //  Knuth S2 and S3
    //

    while (true) {
        //
        //    Base Case:  We've coalesced everything back into the original block.
        //
        if (elem->_kVal == _poolKVal) {
            ESF_ASSERT( ( char * ) elem == _pool );

            pushAvailList(elem);

            break;
        }

        ESFUWord elemAddress = (ESFUWord) (((char *) elem) - _pool);

        if (0 == elemAddress % (ESF_UWORD_C(1) << (elem->_kVal + 1))) {
            //
            // The buddy is to the right of this node.
            //

            buddy = (AvailListElem *) (((char *) elem) + (ESF_UWORD_C(1) << elem->_kVal));

            ESF_ASSERT( buddy->_kVal <= elem->_kVal );

            if (buddy->_tag && buddy->_kVal == elem->_kVal) {
                removeFromAvailList(buddy);

                //
                //  Coalesce two buddies.  When coalescing two buddies, the
                //  address of the left buddy becomes the address of the new
                //  buddy.  Also, the size of the new buddy doubles.
                //

                // elem = elem;

                ++elem->_kVal;

                continue;
            }

            //
            // The buddy node isn't ready to be coalesced so just add
            // the current node to the avail list.
            //

            pushAvailList(elem);

            break;
        }

        //
        // The buddy is to the left of this node.
        //

        buddy = (AvailListElem *) (((char *) elem) - (ESF_UWORD_C(1) << elem->_kVal));

        ESF_ASSERT( buddy->_kVal <= elem->_kVal );

        if (buddy->_tag && buddy->_kVal == elem->_kVal) {
            removeFromAvailList(buddy);

            //
            //    Coalesce two buddies.  When coalescing two buddies, the
            //    address of the left buddy becomes the address of the new
            //    buddy.  Also, the size of the new buddy doubles.
            //

            elem = buddy;

            ++elem->_kVal;

            continue;
        }

        //
        // The buddy node isn't ready to be coalesced so just add
        // the current node to the avail list.
        //

        pushAvailList(elem);

        break;
    }

    return ESF_SUCCESS;
}

ESFUWord ESFBuddyAllocator::getOverhead() {
    return sizeof(AvailListElem);
}

ESFError ESFBuddyAllocator::initialize() {
    if (_pool || 1 > _poolKVal || ESF_AVAIL_LIST_LENGTH <= _poolKVal) {
        return ESF_INVALID_STATE;
    }

    ESFError error = _sourceAllocator->allocate((void **) &_pool, ESF_UWORD_C(1) << _poolKVal);

    if (ESF_SUCCESS != error) {
        return error;
    }

    ESF_ASSERT( _pool );

    AvailListElem *elem = (AvailListElem *) _pool;

    elem->_linkB = 0;
    elem->_linkF = 0;
    elem->_tag = 1;
    elem->_kVal = _poolKVal;

    _availList[_poolKVal] = elem;

    return ESF_SUCCESS;
}

ESFError ESFBuddyAllocator::destroy() {
    if (!_pool) {
        return ESF_INVALID_STATE;
    }

    if (0 == _availList[_poolKVal]) {
        return ESF_IN_USE;
    }

    if (_failoverAllocator) {
        ESFError error = _failoverAllocator->destroy();

        if (ESF_SUCCESS != error) {
            return error;
        }
    }

    _sourceAllocator->deallocate((void *) _pool);
    _pool = 0;

    return ESF_SUCCESS;
}

ESFError ESFBuddyAllocator::isInitialized() {
    return _pool ? ESF_SUCCESS : ESF_NOT_INITIALIZED;
}

ESFError ESFBuddyAllocator::setFailoverAllocator(ESFAllocator *allocator) {
    _failoverAllocator = allocator;

    return ESF_SUCCESS;
}

ESFError ESFBuddyAllocator::getFailoverAllocator(ESFAllocator **allocator) {
    if (!allocator) {
        return ESF_NULL_POINTER;
    }

    *allocator = _failoverAllocator;

    return ESF_SUCCESS;
}
