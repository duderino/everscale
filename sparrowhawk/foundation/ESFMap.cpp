/** @file ESFMap.cpp
 *  @brief A balanced binary tree
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

#ifndef ESF_MAP_H
#include <ESFMap.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifdef DEBUG
#include <math.h>
#endif

ESFMapNode::ESFMapNode(ESFMapNode *parent, ESFMapNode *left, ESFMapNode *right,
		bool isBlack, const void *key, void *value) :
	_parent(parent), _left(left), _right(right), _isBlack(isBlack), _key(key),
			_value(value) {
}

ESFMapNode::~ESFMapNode() {
}

void *
ESFMapNode::operator new(size_t size, ESFAllocator *allocator) {
	return allocator->allocate(size);
}

ESFMap::ESFMap(bool isUnique, ESFComparator *comparator,
		ESFAllocator *allocator, ESFLockable *lockable) :
	_size(0), _isUnique(isUnique), _root(&_sentinel), _allocator(allocator),
			_lockable(lockable), _comparator(comparator),
			_sentinel(&_sentinel, &_sentinel, &_sentinel, true, 0, 0)

{
}

ESFMap::~ESFMap() {
}

ESFError ESFMap::insert(const void *key, void *value) {
	if (!key) {
		return ESF_NULL_POINTER;
	}

	if (!_comparator || !_allocator || !_lockable) {
		return ESF_INVALID_STATE;
	}

	if (ESF_UINT32_MAX == _size) {
		return ESF_OVERFLOW;
	}

	ESFMapNode *node = new (_allocator) ESFMapNode(&_sentinel, &_sentinel,
			&_sentinel, true, key, value);

	if (!node) {
		return ESF_OUT_OF_MEMORY;
	}

	if (false == insertNode(node)) {
		node->~ESFMapNode();
		_allocator->deallocate((void *) node);

		return ESF_UNIQUENESS_VIOLATION;
	}

	return ESF_SUCCESS;
}

ESFError ESFMap::erase(const void *key) {
	if (!key) {
		return ESF_NULL_POINTER;
	}

	if (!_comparator || !_allocator || !_lockable) {
		return ESF_INVALID_STATE;
	}

	ESFMapNode *node = findNode(_root, key);

	if (!node->_key) {
		return ESF_CANNOT_FIND;
	}

	deleteNode(node);

	return ESF_SUCCESS;
}

void *ESFMap::find(const void *key) {
	if (!key) {
		return 0;
	}

	if (!_comparator || !_allocator || !_lockable) {
		return 0;
	}

	ESFMapNode *node = findNode(_root, key);

	if (!node->_key) {
		return 0;
	}

	return node->_value;
}

ESFError ESFMap::update(const void *key, void *value, void **old) {
	if (!key) {
		return ESF_NULL_POINTER;
	}

	if (!_comparator || !_allocator || !_lockable) {
		return ESF_INVALID_STATE;
	}

	ESFMapNode *node = findNode(_root, key);

	if (!node->_key) {
		return ESF_CANNOT_FIND;
	}

	if (old) {
		*old = node->_value;
	}

	node->_value = value;

	return ESF_SUCCESS;
}

ESFError ESFMap::clear() {
	ESFMapNode *x = _root;

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
			ESF_ASSERT(_root == x);

			x->~ESFMapNode();
			_allocator->deallocate((void *) x);

			_root = &_sentinel;

			_size = 0;

			return ESF_SUCCESS;
		}

		if (x == x->_parent->_right) {
			x = x->_parent;

			x->_right->~ESFMapNode();
			_allocator->deallocate((void *) x->_right);

			x->_right = &_sentinel;

			continue;
		}

		ESF_ASSERT(x == x->_parent->_left);

		x = x->_parent;

		x->_left->~ESFMapNode();
		_allocator->deallocate((void *) x->_left);

		x->_left = &_sentinel;
	}

	return ESF_SUCCESS;
}

ESFMapIterator ESFMap::getMinimumIterator() {
	ESFMapIterator iterator(findMinimum(_root));

	return iterator;
}

ESFMapIterator ESFMap::getMaximumIterator() {
	ESFMapIterator iterator(findMaximum(_root));

	return iterator;
}

ESFError ESFMap::insert(const void *key, void *value, ESFMapIterator *iterator) {
	if (!key || !value || !iterator) {
		return ESF_NULL_POINTER;
	}

	if (!_comparator || !_allocator || !_lockable) {
		return ESF_INVALID_STATE;
	}

	ESFMapNode *node = new (_allocator) ESFMapNode(&_sentinel, &_sentinel,
			&_sentinel, true, key, value);

	if (!node) {
		return ESF_OUT_OF_MEMORY;
	}

	if (false == insertNode(node)) {
		node->~ESFMapNode();
		_allocator->deallocate((void *) node);

		return ESF_UNIQUENESS_VIOLATION;
	}

	iterator->_node = node;

	return ESF_SUCCESS;
}

ESFMapIterator ESFMap::findIterator(const void *key) {
	ESFMapIterator iterator;

	if (!key) {
		return iterator;
	}

	if (!_comparator || !_allocator || !_lockable) {
		return iterator;
	}

	ESFMapNode *node = findNode(_root, key);

	if (!node) {
		return iterator;
	}

	iterator._node = node;

	return iterator;
}

ESFError ESFMap::erase(ESFMapIterator *iterator) {
	if (!iterator) {
		return ESF_NULL_POINTER;
	}

	if (iterator->isNull()) {
		return ESF_INVALID_ITERATOR;
	}

	deleteNode(iterator->_node);

	iterator->_node = 0;

	return ESF_SUCCESS;
}

ESFError ESFMap::writeAcquire() {
	if (!_lockable)
		return ESF_INVALID_STATE;

	return _lockable->writeAcquire();
}

ESFError ESFMap::readAcquire() {
	if (!_lockable)
		return ESF_INVALID_STATE;

	return _lockable->readAcquire();
}

ESFError ESFMap::writeAttempt() {
	if (!_lockable)
		return ESF_INVALID_STATE;

	return _lockable->writeAttempt();
}

ESFError ESFMap::readAttempt() {
	if (!_lockable)
		return ESF_INVALID_STATE;

	return _lockable->readAttempt();
}

ESFError ESFMap::writeRelease() {
	if (!_lockable)
		return ESF_INVALID_STATE;

	return _lockable->writeRelease();
}

ESFError ESFMap::readRelease() {
	if (!_lockable)
		return ESF_INVALID_STATE;

	return _lockable->readRelease();
}

ESFMapNode *
ESFMap::findNode(ESFMapNode *x, const void *k) {
	//
	//  See Iterative-Tree-Search in "Introduction to Algorithms", Cormen,
	//  Leiserson, Rivest, p.248.
	//

	int result = 0;

	while (x->_key) {
		result = _comparator->compare(k, x->_key);

		if (0 == result) {
			if (_isUnique) {
				return x;
			}

			//
			//  This is a departure from Cormen et. al.'s algorithm.  If the
			//  map allows multiple elements with the same key, we could have
			//  inserted them to the right of the current node.  We always
			//  return the node that is the "smallest" one in the tree so that
			//  any iteration from this point would yield a different key
			//  if it did a getPrevious() and could yield a node with the
			//  same key if it did a getNext().
			//
			ESFMapNode *y = 0;

			while (true) {
				y = findSuccessor(x);

				if (!y->_key)
					break;

				if (0 != _comparator->compare(y->_key, x->_key))
					break;

				x = y;
			}

			return x;
		} else if (0 < result) {
			x = x->_right;
		} else {
			x = x->_left;
		}
	}

	return &_sentinel;
}

ESFMapNode *
ESFMap::findMinimum(ESFMapNode *x) {
	//
	//  See Tree-Minimum in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 248.
	//

	while (x->_left->_key) {
		x = x->_left;
	}

	return x;
}

ESFMapNode *
ESFMap::findMaximum(ESFMapNode *x) {
	//
	//  See Tree-Maximum in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 248.
	//

	while (x->_right->_key) {
		x = x->_right;
	}

	return x;
}

ESFMapNode *
ESFMap::findSuccessor(ESFMapNode *x) {
	//
	//  See Tree-Successor in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 249.
	//

	if (x->_right->_key) {
		return findMinimum(x->_right);
	}

	ESFMapNode *y = x->_parent;

	while (y->_key && x == y->_right) {
		x = y;
		y = y->_parent;
	}

	return y;
}

ESFMapNode *
ESFMap::findPredecessor(ESFMapNode *x) {
	//
	//  See Tree-Successor in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 249.
	//
	//  Tree-Predecessor is symmetric to Tree-Successor.
	//

	if (x->_left->_key) {
		return findMaximum(x->_left);
	}

	ESFMapNode *y = x->_parent;

	while (y->_key && x == y->_left) {
		x = y;
		y = y->_parent;
	}

	return y;
}

bool ESFMap::insertNode(ESFMapNode *z) {
	//
	//  See Tree-Insert in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 251.
	//

	int result = 0;
	ESFMapNode *y = &_sentinel;
	ESFMapNode *x = _root;

	while (x->_key) {
		y = x;

		result = _comparator->compare(z->_key, x->_key);

		if (0 > result) {
			x = x->_left;
		} else if (0 < result) {
			x = x->_right;
		} else if (_isUnique) {
			return false;
		} else {
			break;
		}
	}

	z->_parent = y;
	z->_left = &_sentinel;
	z->_right = &_sentinel;

	if (&_sentinel == y) {
		_root = z;
	} else if (0 > result) {
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

	while (x != _root && false == x->_parent->_isBlack) {
		if (x->_parent == x->_parent->_parent->_left) {
			y = x->_parent->_parent->_right;

			if (false == y->_isBlack) {
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

			if (false == y->_isBlack) {
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

void ESFMap::deleteNode(ESFMapNode *z) {
	//
	//  See RB-Delete in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 273.
	//

	ESFMapNode *y = 0;
	ESFMapNode *x = 0;

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

	z->~ESFMapNode();
	_allocator->deallocate((void *) z);
	z = 0;

	--_size;

	if (!isBlack)
		return;

	//
	//  See RB-Delete-Fixup in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 274.
	//

	ESFMapNode *w = 0;

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

void ESFMap::rightRotate(ESFMapNode *x) {
	//
	//  See Left-Rotate in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 266.
	//
	//  Right-Rotate is symmetric to Left-Rotate.
	//

	ESFMapNode *y = x->_left;

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

void ESFMap::leftRotate(ESFMapNode *x) {
	//
	//  See Left-Rotate in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 266.
	//

	ESFMapNode *y = x->_right;

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

ESFUInt32 ESFMap::getSize() const {
	return _size;
}

bool ESFMap::isEmpty() const {
	return 0 == _size;
}

#ifdef DEBUG
bool ESFMap::isBalanced() const {
	int height = 0;
	bool unbalanced = false;

	height = getHeight(_root);

	//
	//  See Lemma 14.1 in "Introduction to Algorithms", Cormen, Leiserson,
	//  Rivest, p. 264.
	//
	//  "A red-black tree with n internal nodes has a height at most 2lg(n+1).
	//

	if (height > 2 * (log10(_size + 1) / log10(2))) {
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

int ESFMap::getBlackHeight(ESFMapNode *node, bool *unbalanced) const {
	//
	//  Base case.
	//
	if (!node->_key) {
		ESF_ASSERT( node->_isBlack );
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

int ESFMap::getHeight(ESFMapNode *node) const {
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

#endif  /* defined DEBUG */

ESFSize ESFMap::GetAllocationSize() {
	return sizeof(ESFMapNode);
}

ESFMapIterator::ESFMapIterator() :
	_node(0) {
}

ESFMapIterator::ESFMapIterator(ESFMapNode *node) :
	_node(node) {
}

ESFMapIterator::ESFMapIterator(const ESFMapIterator &iterator) :
	_node(iterator._node) {
}

ESFMapIterator::~ESFMapIterator() {
}

