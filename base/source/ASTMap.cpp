#ifndef ESB_AST_MAP_H
#include <ASTMap.h>
#endif

#ifndef ESB_AST_INTEGER_H
#include <ASTInteger.h>
#endif

#ifndef ESB_AST_DECIMAL_H
#include <ASTDecimal.h>
#endif

#ifndef ESB_AST_STRING_H
#include <ASTString.h>
#endif

namespace ESB {
namespace AST {

Map::JsonMapComparator Map::_Comparator;

Map::Map(Allocator &allocator) : Element(), _map(_Comparator, NullLock::Instance(), allocator) {}

Map::~Map() { clear(); }

Element::Type Map::type() const { return Element::MAP; }

MapIterator Map::iterator() {
  MapIterator it(_map.minimumIterator());
  return it;
}

Error Map::clear() {
  for (MapIterator it = iterator(); !it.isNull(); ++it) {
    Scalar *key = (Scalar *)it.key();
    Element *value = (Element *)it.value();

    if (key && key->cleanupHandler()) {
      key->cleanupHandler()->destroy(key);
    }

    if (value && value->cleanupHandler()) {
      value->cleanupHandler()->destroy(value);
    }
  }

  return _map.clear();
}

int Map::JsonMapComparator::compare(const void *l, const void *r) const {
  const Scalar *left = (const Scalar *)l;
  const Scalar *right = (const Scalar *)r;

  if (left->type() != right->type()) {
    return left->type() - right->type();
  }

  switch (left->type()) {
    case Element::STRING:
      return ((const String *)left)->compare(*((const String *)right));
    case Element::INTEGER:
      return ((const Integer *)left)->compare(*((const Integer *)right));
    case Element::DECIMAL:
      return ((const Decimal *)left)->compare(*((const Decimal *)right));
    default:
      assert(!"Invalid json scalar type");
      return -1;
  }
}

MapIterator::MapIterator() : _it() {}

MapIterator::MapIterator(const MapIterator &it) : _it(it._it) {}

MapIterator::~MapIterator() {}

MapIterator::MapIterator(const ESB::MapIterator &it) : _it(it) {}

}  // namespace AST
}  // namespace ESB
