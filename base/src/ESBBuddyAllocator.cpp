/** @file ESBBuddyAllocator.cpp
 *  @brief A ESBAllocator implementation good for variable-length allocations
 */

#ifndef ESB_BUDDY_ALLOCATOR_H
#include <ESBBuddyAllocator.h>
#endif

#ifndef ESB_ASSERT_H
#include <ESBAssert.h>
#endif

namespace ESB {

BuddyAllocator::KVal BuddyAllocator::GetKVal(UWord requestedSize) {
  UWord adjustedSize = requestedSize + sizeof(AvailListElem);
  KVal kVal = 1;

  while (adjustedSize > (ESB_UWORD_C(1) << kVal)) {
    ++kVal;
  }

  return kVal;
}

BuddyAllocator::AvailListElem *BuddyAllocator::popAvailList(
    BuddyAllocator::KVal kVal) {
  ESB_ASSERT(kVal < ESB_AVAIL_LIST_LENGTH);

  AvailListElem *elem = _availList[kVal];

  if (!elem) return 0;

  AvailListElem *next = elem->_linkF;

  ESB_ASSERT(0 == elem->_linkB);
  ESB_ASSERT(kVal == elem->_kVal);
  ESB_ASSERT(1 == elem->_tag);

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

void BuddyAllocator::removeFromAvailList(BuddyAllocator::AvailListElem *elem) {
  ESB_ASSERT(elem);
  ESB_ASSERT(elem->_kVal < ESB_AVAIL_LIST_LENGTH);

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

void BuddyAllocator::pushAvailList(AvailListElem *elem) {
  ESB_ASSERT(elem);
  ESB_ASSERT(elem->_kVal < ESB_AVAIL_LIST_LENGTH);

  AvailListElem *next = _availList[elem->_kVal];

  elem->_linkB = 0;
  elem->_linkF = next;
  elem->_tag = 1;

  if (next) {
    next->_linkB = elem;
  }

  _availList[elem->_kVal] = elem;
}

BuddyAllocator::BuddyAllocator(int size, Allocator *source)
    : _failoverAllocator(0), _sourceAllocator(source), _pool(0), _poolKVal(0) {
  if (1 > size || ESB_AVAIL_LIST_LENGTH < size) return;

  if (!_sourceAllocator) return;

  for (int i = 0; i < ESB_AVAIL_LIST_LENGTH; ++i) {
    _availList[i] = 0;
  }

  _poolKVal = size;
  _sourceAllocator = source;
}

BuddyAllocator::~BuddyAllocator() { destroy(); }

void *BuddyAllocator::allocate(UWord size) {
  if (0 == size) {
    return 0;
  }

  if (!_pool) {
    if (ESB_SUCCESS != initialize()) {
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

  for (; actualKVal < ESB_AVAIL_LIST_LENGTH; ++actualKVal) {
    if (0 != _availList[actualKVal]) {
      elem = (char *)popAvailList(actualKVal);
      break;
    }
  }

  if (!elem) {
    if (!_failoverAllocator) return 0;

    return _failoverAllocator->allocate(size);
  }

  //
  //    Keep splitting the elem until we are left with a minimally sized elem.
  //
  //    Knuth R3 and R4
  //

  ESB_ASSERT(actualKVal >= minimumKVal);

  AvailListElem *right = (AvailListElem *)elem;
  AvailListElem *left = 0;

  while (minimumKVal < actualKVal) {
    --actualKVal;

    left = (AvailListElem *)(elem + (ESB_UWORD_C(1) << actualKVal));

    right->_kVal = actualKVal;

    left->_linkB = 0;
    left->_linkF = 0;
    left->_kVal = actualKVal;
    left->_tag = 0;

    pushAvailList(left);
  }

  ESB_ASSERT(right);

  return (void *)(((char *)right) + sizeof(AvailListElem));
}

Error BuddyAllocator::allocate(void **block, UWord size) {
  if (!block) {
    return ESB_NULL_POINTER;
  }

  if (0 == size) {
    return ESB_INVALID_ARGUMENT;
  }

  if (!_pool) {
    Error error = initialize();

    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  void *tmp = allocate(size);

  if (!tmp) {
    return ESB_OUT_OF_MEMORY;
  }

  *block = tmp;

  return ESB_SUCCESS;
}

Error BuddyAllocator::deallocate(void *block) {
  if (!block) {
    return ESB_NULL_POINTER;
  }

  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  char *trueAddress = ((char *)block) - sizeof(AvailListElem);

  if (trueAddress < (char *)_pool ||
      trueAddress >= (char *)_pool + (ESB_UWORD_C(1) << _poolKVal)) {
    if (_failoverAllocator) {
      return _failoverAllocator->deallocate(block);
    }

    return ESB_NOT_OWNER;
  }

  //
  //    Find this block's buddy and check its availability.
  //
  //    Knuth S1
  //

  AvailListElem *elem = (AvailListElem *)trueAddress;
  AvailListElem *buddy = 0;

  ESB_ASSERT(0 == elem->_linkB);
  ESB_ASSERT(0 == elem->_linkF);
  ESB_ASSERT(0 == elem->_tag);

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
      ESB_ASSERT((char *)elem == _pool);

      pushAvailList(elem);

      break;
    }

    UWord elemAddress = (UWord)(((char *)elem) - (char *)_pool);

    if (0 == elemAddress % (ESB_UWORD_C(1) << (elem->_kVal + 1))) {
      //
      // The buddy is to the right of this node.
      //

      buddy =
          (AvailListElem *)(((char *)elem) + (ESB_UWORD_C(1) << elem->_kVal));

      ESB_ASSERT(buddy->_kVal <= elem->_kVal);

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

    buddy = (AvailListElem *)(((char *)elem) - (ESB_UWORD_C(1) << elem->_kVal));

    ESB_ASSERT(buddy->_kVal <= elem->_kVal);

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

  return ESB_SUCCESS;
}

UWord BuddyAllocator::getOverhead() { return sizeof(AvailListElem); }

Error BuddyAllocator::initialize() {
  if (_pool || 1 > _poolKVal || _AVAIL_LIST_LENGTH <= _poolKVal) {
    return ESB_INVALID_STATE;
  }

  _pool = _sourceAllocator->allocate(ESB_UWORD_C(1) << _poolKVal);

  if (0 == _pool) {
    return ESB_OUT_OF_MEMORY;
  }

  ESB_ASSERT(_pool);

  AvailListElem *elem = (AvailListElem *)_pool;

  elem->_linkB = 0;
  elem->_linkF = 0;
  elem->_tag = 1;
  elem->_kVal = _poolKVal;

  _availList[_poolKVal] = elem;

  return ESB_SUCCESS;
}

Error BuddyAllocator::destroy() {
  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  if (0 == _availList[_poolKVal]) {
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

Error BuddyAllocator::isInitialized() {
  return _pool ? ESB_SUCCESS : ESB_NOT_INITIALIZED;
}

Error BuddyAllocator::setFailoverAllocator(Allocator *allocator) {
  _failoverAllocator = allocator;

  return ESB_SUCCESS;
}

Error BuddyAllocator::getFailoverAllocator(Allocator **allocator) {
  if (!allocator) {
    return ESB_NULL_POINTER;
  }

  *allocator = _failoverAllocator;

  return ESB_SUCCESS;
}

}  // namespace ESB
