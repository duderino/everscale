#ifndef ESB_BUDDY_ALLOCATOR_H
#include <ESBBuddyAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

BuddyAllocator::KVal BuddyAllocator::UWordToKVal(UWord uWord) {
  // This is a non-recursive binary search that rounds up to the nearest power of 2

  KVal min = 0;
  KVal kVal = 8;  // best guess starting point of 2^8 = 256 adjusted sizes
  KVal max = ESB_AVAIL_LIST_LENGTH;

  while (true) {
    if (uWord > (ESB_UWORD_C(1) << kVal)) {
      // kVal too small, move it halfway to max
      min = kVal;
      kVal = kVal + (max - kVal) / 2;
    } else {
      if (uWord > (ESB_UWORD_C(1) << (kVal - 1))) {
        break;
      }
      // kVal too large, move it halfway to min
      max = kVal;
      kVal = kVal - (kVal - min) / 2;
    }
  }

  return kVal;
}

BuddyAllocator::AvailListElem *BuddyAllocator::popAvailList(BuddyAllocator::KVal kVal) {
  assert(kVal < ESB_AVAIL_LIST_LENGTH);

  AvailListElem *elem = _availList[kVal];

  if (!elem) return 0;

  AvailListElem *next = elem->_linkF;

  assert(0 == elem->_linkB);
  assert(kVal == elem->_kVal);
  assert(1 == elem->_tag);

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
  assert(elem);
  assert(elem->_kVal < ESB_AVAIL_LIST_LENGTH);

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
  assert(elem);
  assert(elem->_kVal < ESB_AVAIL_LIST_LENGTH);

  AvailListElem *next = _availList[elem->_kVal];

  elem->_linkB = 0;
  elem->_linkF = next;
  elem->_tag = 1;

  if (next) {
    next->_linkB = elem;
  }

  _availList[elem->_kVal] = elem;
}

BuddyAllocator::BuddyAllocator(UInt32 size, Allocator &source)
    : _sourceAllocator(source), _pool(0), _poolKVal(0), _cleanupHandler(*this) {
  for (int i = 0; i < ESB_AVAIL_LIST_LENGTH; ++i) {
    _availList[i] = NULL;
  }

  // Round size up to the nearest power of 2
  _poolKVal = UWordToKVal(size);
}

BuddyAllocator::~BuddyAllocator() { reset(); }

Error BuddyAllocator::allocate(UWord size, void **block) {
#ifdef ESB_NO_ALLOC
  return SystemAllocator::Instance().allocate(size, block);
#else
  if (0 == size) {
    return ESB_INVALID_ARGUMENT;
  }

  if (!block) {
    return ESB_NULL_POINTER;
  }

  if (!_pool) {
    Error error = initialize();
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  //
  //    Find an element large enough to fulfill the request.  If found, remove
  //    it from the available list.  If not found, try the failover allocator.
  //
  //    Knuth R1 and R2
  //

  KVal minimumKVal = AdjustedKVal(size);
  KVal actualKVal = minimumKVal;
  char *elem = NULL;

  for (; actualKVal < ESB_AVAIL_LIST_LENGTH; ++actualKVal) {
    if (_availList[actualKVal]) {
      elem = (char *)popAvailList(actualKVal);
      break;
    }
  }

  if (!elem) {
    return ESB_OUT_OF_MEMORY;
  }

  //
  //    Keep splitting the elem until we are left with a minimally sized elem.
  //
  //    Knuth R3 and R4
  //

  assert(actualKVal >= minimumKVal);

  AvailListElem *right = (AvailListElem *)elem;
  AvailListElem *left = NULL;

  while (minimumKVal < actualKVal) {
    --actualKVal;

    right->_kVal = actualKVal;

    left = (AvailListElem *)(elem + (ESB_UWORD_C(1) << actualKVal));
    left->_linkB = NULL;
    left->_linkF = NULL;
    left->_kVal = actualKVal;
    left->_tag = 0;
    pushAvailList(left);
  }

  assert(right);
  *block = (void *)(((char *)right) + sizeof(AvailListElem));
  return ESB_SUCCESS;
#endif
}

Error BuddyAllocator::deallocate(void *block) {
#ifdef ESB_NO_ALLOC
  return SystemAllocator::Instance().deallocate(block);
#else
  if (!block) {
    return ESB_NULL_POINTER;
  }

  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  if (!owns(block)) {
    return ESB_NOT_OWNER;
  }

  //
  //    Find this block's buddy and check its availability.
  //
  //    Knuth S1
  //

  AvailListElem *elem = (AvailListElem *)(((char *)block) - sizeof(AvailListElem));
  AvailListElem *buddy = NULL;

  assert(!elem->_linkB);
  assert(!elem->_linkF);
  assert(!elem->_tag);

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
      assert((char *)elem == _pool);
      pushAvailList(elem);
      break;
    }

    UWord elemAddress = (UWord)(((char *)elem) - (char *)_pool);

    if (0 == elemAddress % (ESB_UWORD_C(1) << (elem->_kVal + 1))) {
      //
      // The buddy is to the right of this node.
      //

      buddy = (AvailListElem *)(((char *)elem) + (ESB_UWORD_C(1) << elem->_kVal));
      assert(buddy->_kVal <= elem->_kVal);

      if (buddy->_tag && buddy->_kVal == elem->_kVal) {
        //
        //  Coalesce two buddies.  When coalescing two buddies, the
        //  address of the left buddy becomes the address of the new
        //  buddy.  Also, the size of the new buddy doubles.
        //

        removeFromAvailList(buddy);
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
    assert(buddy->_kVal <= elem->_kVal);

    if (buddy->_tag && buddy->_kVal == elem->_kVal) {
      //
      //    Coalesce two buddies.  When coalescing two buddies, the
      //    address of the left buddy becomes the address of the new
      //    buddy.  Also, the size of the new buddy doubles.
      //

      removeFromAvailList(buddy);
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
#endif
}

Error BuddyAllocator::initialize() {
  if (_pool || 1 > _poolKVal || ESB_AVAIL_LIST_LENGTH <= _poolKVal) {
    return ESB_INVALID_STATE;
  }

  Error error = _sourceAllocator.allocate(ESB_UWORD_C(1) << _poolKVal, &_pool);
  if (ESB_SUCCESS != error) {
    return error;
  }

  assert(_pool);

  AvailListElem *elem = (AvailListElem *)_pool;

  elem->_linkB = NULL;
  elem->_linkF = NULL;
  elem->_tag = 1;
  elem->_kVal = _poolKVal;

  _availList[_poolKVal] = elem;

  return ESB_SUCCESS;
}

Error BuddyAllocator::reset() {
#ifdef ESB_NO_ALLOC
  return ESB_SUCCESS;
#else
  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  if (!_availList[_poolKVal]) {
    return ESB_IN_USE;
  }

  _sourceAllocator.deallocate(_pool);
  _pool = NULL;

  return ESB_SUCCESS;
#endif
}

CleanupHandler &BuddyAllocator::cleanupHandler() { return _cleanupHandler; }

bool BuddyAllocator::reallocates() { return true; }

Error BuddyAllocator::reallocate(void *oldBlock, UWord size, void **newBlock) {
#ifdef ESB_NO_ALLOC
  return SystemAllocator::Instance().reallocate(oldBlock, size, newBlock);
#else
  if (!oldBlock) {
    return allocate(size, newBlock);
  }

  if (0 == size) {
    return deallocate(oldBlock);
  }

  if (!_pool) {
    return ESB_INVALID_STATE;
  }

  UWord originalSize = allocationSize(oldBlock);
  if (0 == originalSize) {
    return ESB_NOT_OWNER;
  }

  if (size <= originalSize) {
    // TODO return memory if possible
    *newBlock = oldBlock;
    return ESB_SUCCESS;
  }

  KVal maxKVal = AdjustedKVal(size);
  void *block = NULL;
  Error error = reallocateInternal(oldBlock, &block, maxKVal, true);

  // First try to grow the current block by coalescing buddies

  if (ESB_SUCCESS == error) {
    error = reallocateInternal(oldBlock, &block, maxKVal, false);
    assert(ESB_SUCCESS == error);
    if (ESB_SUCCESS != error) {
      return error;
    }
    if (oldBlock != block) {
      memmove(block, oldBlock, MIN(originalSize, size));
    }
    *newBlock = block;
    return ESB_SUCCESS;
  }

  // Failing that, try to allocate a new block of sufficient size

  error = allocate(size, &block);
  if (ESB_SUCCESS != error) {
    return error;
  }

  // Copy data from the old block, then free the old block

  memcpy(block, oldBlock, MIN(originalSize, size));
  deallocate(oldBlock);
  *newBlock = block;
  return ESB_SUCCESS;
#endif
}

Error BuddyAllocator::reallocateInternal(void *oldBlock, void **newBlock, BuddyAllocator::KVal maxKVal, bool dryRun) {
  //
  //    Find this block's buddy and check its availability.
  //
  //    Knuth S1
  //

  AvailListElem *elem = (AvailListElem *)(((char *)oldBlock) - sizeof(AvailListElem));
  AvailListElem *buddy = NULL;

  assert(!elem->_linkB);
  assert(!elem->_linkF);
  assert(!elem->_tag);

  elem->_dryRunKVal = elem->_kVal;

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
    if (dryRun) {
      if (elem->_dryRunKVal == maxKVal) {
        return ESB_SUCCESS;
      }
    } else {
      if (elem->_kVal == maxKVal) {
        elem->_tag = 0;
        *newBlock = (void *)(((char *)elem) + sizeof(AvailListElem));
        return ESB_SUCCESS;
      }
    }

    UWord elemAddress = (UWord)(((char *)elem) - (char *)_pool);
    KVal currentKVal = dryRun ? elem->_dryRunKVal : elem->_kVal;

    if (0 == elemAddress % (ESB_UWORD_C(1) << (elem->_kVal + 1))) {
      //
      // The buddy is to the right of this node.
      //

      buddy = (AvailListElem *)(((char *)elem) + (ESB_UWORD_C(1) << elem->_kVal));
      assert(buddy->_kVal <= currentKVal);

      if (buddy->_tag && buddy->_kVal == currentKVal) {
        //
        //  Coalesce two buddies.  When coalescing two buddies, the
        //  address of the left buddy becomes the address of the new
        //  buddy (a no-op in this case).  Also, the size of the new buddy
        //  doubles.
        //

        if (dryRun) {
          ++elem->_dryRunKVal;
        } else {
          removeFromAvailList(buddy);
          ++elem->_kVal;
        }
        continue;
      }

      //
      // The buddy node isn't available so the allocated chunk cannot grow.
      //

      return ESB_OUT_OF_MEMORY;
    }

    //
    // The buddy is to the left of this node.
    //

    buddy = (AvailListElem *)(((char *)elem) - (ESB_UWORD_C(1) << elem->_kVal));
    assert(buddy->_kVal <= currentKVal);

    if (buddy->_tag && buddy->_kVal == currentKVal) {
      //
      //    Coalesce two buddies.  When coalescing two buddies, the
      //    address of the left buddy becomes the address of the new
      //    buddy.  Also, the size of the new buddy doubles.
      //

      buddy->_dryRunKVal = currentKVal;
      elem = buddy;
      if (dryRun) {
        ++elem->_dryRunKVal;
      } else {
        removeFromAvailList(buddy);
        ++elem->_kVal;
      }
      continue;
    }

    //
    // The buddy node isn't available so the allocated chunk cannot grow.
    //

    return ESB_OUT_OF_MEMORY;
  }
}

UWord BuddyAllocator::allocationSize(void *block) const {
  if (!owns(block)) {
    return 0;
  }

  AvailListElem *elem = (AvailListElem *)(((char *)block) - sizeof(AvailListElem));
  return (ESB_UWORD_C(1) << elem->_kVal) - sizeof(AvailListElem);
}

}  // namespace ESB
