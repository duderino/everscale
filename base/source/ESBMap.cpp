#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

#include <math.h>

namespace ESB {

MapNode::MapNode(MapNode *parent, MapNode *left, MapNode *right, bool isBlack,
                 const void *key, void *value)
    : _parent(parent),
      _left(left),
      _right(right),
      _isBlack(isBlack),
      _key(key),
      _value(value) {}

MapNode::~MapNode() {}

Map::Map(Comparator &comparator, Lockable &lockable, Allocator &allocator)
    : _size(0),
      _root(&_sentinel),
      _allocator(allocator),
      _lockable(lockable),
      _comparator(comparator),
      _sentinel(&_sentinel, &_sentinel, &_sentinel, true, NULL, NULL) {}

Map::~Map() { clear(); }

Error Map::insert(const void *key, void *value) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  if (ESB_UINT32_MAX == _size) {
    return ESB_OVERFLOW;
  }

  MapNode *node = new (_allocator)
      MapNode(&_sentinel, &_sentinel, &_sentinel, true, key, value);

  if (!node) {
    return ESB_OUT_OF_MEMORY;
  }

  if (!insertNode(node)) {
    node->~MapNode();
    _allocator.deallocate((void *)node);

    return ESB_UNIQUENESS_VIOLATION;
  }

  return ESB_SUCCESS;
}

Error Map::remove(const void *key) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  MapNode *node = findNode(_root, key);

  if (!node->_key) {
    return ESB_CANNOT_FIND;
  }

  deleteNode(node);

  return ESB_SUCCESS;
}

void *Map::find(const void *key) {
  if (!key) {
    return NULL;
  }

  MapNode *node = findNode(_root, key);

  if (!node->_key) {
    return NULL;
  }

  return node->_value;
}

Error Map::update(const void *key, void *value, void **old) {
  if (!key) {
    return ESB_NULL_POINTER;
  }

  MapNode *node = findNode(_root, key);

  if (!node->_key) {
    return ESB_CANNOT_FIND;
  }

  if (old) {
    *old = node->_value;
  }

  node->_value = value;

  return ESB_SUCCESS;
}

Error Map::clear() {
  MapNode *x = _root;

  while (x->_key) {
    if (x->_right->_key) {
      x = x->_right;

      continue;
    }

    if (x->_left->_key) {
      x = x->_left;

      continue;
    }

    if (!x->_parent->_key) {
      assert(_root == x);

      x->~MapNode();
      _allocator.deallocate((void *)x);

      _root = &_sentinel;

      _size = 0;

      return ESB_SUCCESS;
    }

    if (x == x->_parent->_right) {
      x = x->_parent;

      x->_right->~MapNode();
      _allocator.deallocate((void *)x->_right);

      x->_right = &_sentinel;

      continue;
    }

    assert(x == x->_parent->_left);

    x = x->_parent;

    x->_left->~MapNode();
    _allocator.deallocate((void *)x->_left);

    x->_left = &_sentinel;
  }

  return ESB_SUCCESS;
}

MapIterator Map::minimumIterator() {
  MapIterator iterator(findMinimum(_root));
  return iterator;
}

MapIterator Map::maximumIterator() {
  MapIterator iterator(findMaximum(_root));
  return iterator;
}

Error Map::insert(const void *key, void *value, MapIterator *iterator) {
  if (!key || !value || !iterator) {
    return ESB_NULL_POINTER;
  }

  MapNode *node = new (_allocator)
      MapNode(&_sentinel, &_sentinel, &_sentinel, true, key, value);

  if (!node) {
    return ESB_OUT_OF_MEMORY;
  }

  if (false == insertNode(node)) {
    node->~MapNode();
    _allocator.deallocate((void *)node);
    return ESB_UNIQUENESS_VIOLATION;
  }

  iterator->_node = node;

  return ESB_SUCCESS;
}

MapIterator Map::findIterator(const void *key) {
  MapIterator iterator;

  if (!key) {
    return iterator;
  }

  MapNode *node = findNode(_root, key);

  if (!node) {
    return iterator;
  }

  iterator._node = node;

  return iterator;
}

Error Map::erase(MapIterator *iterator) {
  if (!iterator) {
    return ESB_NULL_POINTER;
  }

  if (iterator->isNull()) {
    return ESB_INVALID_ITERATOR;
  }

  deleteNode(iterator->_node);

  iterator->_node = NULL;

  return ESB_SUCCESS;
}

Error Map::writeAcquire() { return _lockable.writeAcquire(); }

Error Map::readAcquire() { return _lockable.readAcquire(); }

Error Map::writeAttempt() { return _lockable.writeAttempt(); }

Error Map::readAttempt() { return _lockable.readAttempt(); }

Error Map::writeRelease() { return _lockable.writeRelease(); }

Error Map::readRelease() { return _lockable.readRelease(); }

MapNode *Map::findNode(MapNode *x, const void *k) {
  //
  //  See Iterative-Tree-Search in "Introduction to Algorithms", Cormen,
  //  Leiserson, Rivest, p.248.
  //

  int result = 0;

  while (x->_key) {
    result = _comparator.compare(k, x->_key);

    if (0 == result) {
      return x;
    } else if (0 < result) {
      x = x->_right;
    } else {
      x = x->_left;
    }
  }

  return &_sentinel;
}

MapNode *Map::findMinimum(MapNode *x) {
  //
  //  See Tree-Minimum in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 248.
  //

  while (x->_left->_key) {
    x = x->_left;
  }

  return x;
}

MapNode *Map::findMaximum(MapNode *x) {
  //
  //  See Tree-Maximum in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 248.
  //

  while (x->_right->_key) {
    x = x->_right;
  }

  return x;
}

MapNode *Map::findSuccessor(MapNode *x) {
  //
  //  See Tree-Successor in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 249.
  //

  if (x->_right->_key) {
    return findMinimum(x->_right);
  }

  MapNode *y = x->_parent;

  while (y->_key && x == y->_right) {
    x = y;
    y = y->_parent;
  }

  return y;
}

MapNode *Map::findPredecessor(MapNode *x) {
  //
  //  See Tree-Successor in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 249.
  //
  //  Tree-Predecessor is symmetric to Tree-Successor.
  //

  if (x->_left->_key) {
    return findMaximum(x->_left);
  }

  MapNode *y = x->_parent;

  while (y->_key && x == y->_left) {
    x = y;
    y = y->_parent;
  }

  return y;
}

bool Map::insertNode(MapNode *z) {
  //
  //  See Tree-Insert in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 251.
  //

  int result = 0;
  MapNode *y = &_sentinel;
  MapNode *x = _root;

  while (x->_key) {
    y = x;

    result = _comparator.compare(z->_key, x->_key);

    if (0 > result) {
      x = x->_left;
    } else if (0 < result) {
      x = x->_right;
    } else {
      return false;
    }
  }

  z->_parent = y;
  z->_left = &_sentinel;
  z->_right = &_sentinel;

  if (&_sentinel == y) {
    _root = z;
  } else if (0 >= result) {
    y->_left = z;
  } else {
    y->_right = z;
  }

  ++_size;

  //
  //  See RB-Insert in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 268.
  //

  x = z;

  x->_isBlack = false;

  while (x != _root && !x->_parent->_isBlack) {
    if (x->_parent == x->_parent->_parent->_left) {
      y = x->_parent->_parent->_right;

      if (!y->_isBlack) {
        x->_parent->_isBlack = true;
        y->_isBlack = true;
        x->_parent->_parent->_isBlack = false;
        x = x->_parent->_parent;
      } else if (x == x->_parent->_right) {
        x = x->_parent;
        leftRotate(x);
      } else {
        x->_parent->_isBlack = true;
        x->_parent->_parent->_isBlack = false;
        rightRotate(x->_parent->_parent);
      }
    } else {
      y = x->_parent->_parent->_left;

      if (!y->_isBlack) {
        x->_parent->_isBlack = true;
        y->_isBlack = true;
        x->_parent->_parent->_isBlack = false;
        x = x->_parent->_parent;
      } else if (x == x->_parent->_left) {
        x = x->_parent;
        rightRotate(x);
      } else {
        x->_parent->_isBlack = true;
        x->_parent->_parent->_isBlack = false;
        leftRotate(x->_parent->_parent);
      }
    }
  }

  _root->_isBlack = true;

  return true;
}

void Map::deleteNode(MapNode *z) {
  //
  //  See RB-Delete in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 273.
  //

  MapNode *y = NULL;
  MapNode *x = NULL;

  if (!z->_left->_key || !z->_right->_key) {
    y = z;
  } else {
    y = findSuccessor(z);
  }

  if (y->_left->_key) {
    x = y->_left;
  } else {
    x = y->_right;
  }

  x->_parent = y->_parent;

  if (!y->_parent->_key) {
    _root = x;
  } else if (y == y->_parent->_left) {
    y->_parent->_left = x;
  } else {
    y->_parent->_right = x;
  }

  bool isBlack = y->_isBlack;

  if (y != z) {
    //
    //  Cormen et. al. simplify this step by copying all fields from
    //  y into z, but since this would violate our iterator invalidation
    //  rules, we really have to replace node z with node y.
    //

    //  Replace all of y's links with z's

    y->_parent = z->_parent;
    y->_left = z->_left;
    y->_right = z->_right;
    y->_isBlack = z->_isBlack;

    //  Change the all the links that point to z to instead point to y.

    y->_right->_parent = y;
    y->_left->_parent = y;

    if (z == y->_parent->_right) {
      y->_parent->_right = y;
    } else {
      y->_parent->_left = y;
    }

    //  If z was the root, y is now the root.

    if (_root == z) {
      _root = y;
    }
  }

  z->~MapNode();
  _allocator.deallocate((void *)z);
  z = NULL;

  --_size;

  if (!isBlack) return;

  //
  //  See RB-Delete-Fixup in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 274.
  //

  MapNode *w = NULL;

  while (_root != x && x->_isBlack) {
    if (x == x->_parent->_left) {
      w = x->_parent->_right;

      if (!w->_isBlack) {
        w->_isBlack = true;
        x->_parent->_isBlack = false;
        leftRotate(x->_parent);
        w = x->_parent->_right;
      }

      if (w->_left->_isBlack && w->_right->_isBlack) {
        w->_isBlack = false;
        x = x->_parent;
      } else if (w->_right->_isBlack) {
        w->_left->_isBlack = true;
        w->_isBlack = false;
        rightRotate(w);
        w = x->_parent->_right;
      } else {
        w->_isBlack = x->_parent->_isBlack;
        x->_parent->_isBlack = true;
        w->_right->_isBlack = true;
        leftRotate(x->_parent);
        x = _root;
      }
    } else {
      w = x->_parent->_left;

      if (!w->_isBlack) {
        w->_isBlack = true;
        x->_parent->_isBlack = false;
        rightRotate(x->_parent);
        w = x->_parent->_left;
      }

      if (w->_right->_isBlack && w->_left->_isBlack) {
        w->_isBlack = false;
        x = x->_parent;
      } else if (w->_left->_isBlack) {
        w->_right->_isBlack = true;
        w->_isBlack = false;
        leftRotate(w);
        w = x->_parent->_left;
      } else {
        w->_isBlack = x->_parent->_isBlack;
        x->_parent->_isBlack = true;
        w->_left->_isBlack = true;
        rightRotate(x->_parent);
        x = _root;
      }
    }
  }

  x->_isBlack = true;
}

void Map::rightRotate(MapNode *x) {
  //
  //  See Left-Rotate in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 266.
  //
  //  Right-Rotate is symmetric to Left-Rotate.
  //

  MapNode *y = x->_left;

  x->_left = y->_right;

  if (y->_right->_key) {
    y->_right->_parent = x;
  }

  y->_parent = x->_parent;

  if (!x->_parent->_key) {
    _root = y;
  } else if (x == x->_parent->_right) {
    x->_parent->_right = y;
  } else {
    x->_parent->_left = y;
  }

  y->_right = x;
  x->_parent = y;
}

void Map::leftRotate(MapNode *x) {
  //
  //  See Left-Rotate in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 266.
  //

  MapNode *y = x->_right;

  x->_right = y->_left;

  if (y->_left->_key) {
    y->_left->_parent = x;
  }

  y->_parent = x->_parent;

  if (!x->_parent->_key) {
    _root = y;
  } else if (x == x->_parent->_left) {
    x->_parent->_left = y;
  } else {
    x->_parent->_right = y;
  }

  y->_left = x;
  x->_parent = y;
}

UInt32 Map::size() const { return _size; }

bool Map::isBalanced() const {
  int height = 0;
  bool unbalanced = false;

  height = getHeight(_root);

  //
  //  See Lemma 14.1 in "Introduction to Algorithms", Cormen, Leiserson,
  //  Rivest, p. 264.
  //
  //  "A red-black tree with n internal nodes has a height at most 2lg(n+1).
  //

  if (height > 2 * (std::log10(_size + 1) / std::log10(2))) {
    return false;
  }

  //
  //  See red-black properties in "Introduction to Algorithms", Cormen,
  //  Leiserson, Rivest, p. 263.
  //
  //  "Every simple path from a node to a descendant leaf contains the
  //  same number of black nodes."
  //

  getBlackHeight(_root, &unbalanced);

  return !unbalanced;
}

int Map::getBlackHeight(MapNode *node, bool *unbalanced) const {
  //
  //  Base case.
  //
  if (!node->_key) {
    assert(node->_isBlack);
    return 0;
  }

  int right = getBlackHeight(node->_right, unbalanced);
  int left = getBlackHeight(node->_left, unbalanced);

  if (right != left) {
    *unbalanced = true;
  }

  if (node->_isBlack) {
    return (right > left) ? right + 1 : left + 1;
  }

  return (right > left) ? right : left;
}

int Map::getHeight(MapNode *node) const {
  //
  //  Base case.
  //
  if (!node->_key) {
    return 0;
  }

  int right = getHeight(node->_right);
  int left = getHeight(node->_left);

  return (right > left) ? right + 1 : left + 1;
}

Size Map::AllocationSize() { return sizeof(MapNode); }

MapIterator::MapIterator() : _node(NULL) {}

MapIterator::MapIterator(MapNode *node) : _node(node) {}

MapIterator::MapIterator(const MapIterator &iterator) : _node(iterator._node) {}

MapIterator::~MapIterator() {}

}  // namespace ESB
