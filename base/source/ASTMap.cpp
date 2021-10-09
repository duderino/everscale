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

#ifndef ESB_AST_BOOLEAN_H
#include <ASTBoolean.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
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

Element *Map::find(const char *key) {
  String str(_allocator, false);
  str.setValue(key);
  return find(&str);
}

const Element *Map::find(const char *key) const {
  String str(_allocator, false);
  str.setValue(key);
  return find(&str);
}

Error Map::find(const char *key, const char **value, bool optional) const {
  if (!key || !value) {
    return ESB_NULL_POINTER;
  }

  *value = NULL;
  const AST::String *tmp = NULL;

  switch (Error error = find(key, (const AST::Element **)&tmp, AST::Element::STRING)) {
    case ESB_SUCCESS:
      *value = tmp->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::findAndDuplicate(Allocator &allocator, const char *key, char **duplicate, bool optional) const {
  if (!key || !duplicate) {
    return ESB_NULL_POINTER;
  }

  *duplicate = NULL;
  const char *value = NULL;
  Error error = find(key, &value, false);

  switch (error) {
    case ESB_SUCCESS:
      return Duplicate(value, allocator, duplicate);
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, UInt16 *value, bool optional) const {
  const AST::Integer *integer = NULL;
  switch (Error error = find(key, &integer, ESB_UINT16_MIN, ESB_UINT16_MAX)) {
    case ESB_SUCCESS:
      *value = integer->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, Int16 *value, bool optional) const {
  const AST::Integer *integer = NULL;
  switch (Error error = find(key, &integer, ESB_INT16_MIN, ESB_INT16_MAX)) {
    case ESB_SUCCESS:
      *value = integer->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, UInt32 *value, bool optional) const {
  const AST::Integer *integer = NULL;
  switch (Error error = find(key, &integer, ESB_UINT32_MIN, ESB_UINT32_MAX)) {
    case ESB_SUCCESS:
      *value = integer->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, Int32 *value, bool optional) const {
  const AST::Integer *integer = NULL;
  switch (Error error = find(key, &integer, ESB_INT32_MIN, ESB_INT32_MAX)) {
    case ESB_SUCCESS:
      *value = integer->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, Int64 *value, bool optional) const {
  const AST::Integer *integer = NULL;
  switch (Error error = find(key, &integer, ESB_INT64_MIN, ESB_INT64_MAX)) {
    case ESB_SUCCESS:
      *value = integer->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, double *value, bool optional) const {
  const AST::Decimal *decimal = NULL;
  switch (Error error = find(key, (const AST::Element **)&decimal, AST::Element::DECIMAL)) {
    case ESB_SUCCESS:
      *value = decimal->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, bool *value, bool optional) const {
  const AST::Boolean *boolean = NULL;
  switch (Error error = find(key, (const AST::Element **)&boolean, AST::Element::BOOLEAN)) {
    case ESB_SUCCESS:
      *value = boolean->value();
      return ESB_SUCCESS;
    case ESB_MISSING_FIELD:
      return optional ? ESB_SUCCESS : ESB_MISSING_FIELD;
    default:
      return error;
  }
}

Error Map::find(const char *key, UniqueId &uuid) const {
  const char *value = NULL;
  Error error = find(key, &value);
  if (ESB_SUCCESS != error) {
    return error;
  }
  assert(value);

  UInt128 id = 0;
  error = ESB::UniqueId::Parse(value, &id);
  if (ESB_SUCCESS != error) {
    return error;
  }

  uuid.set(id);
  return ESB_SUCCESS;
}

Error Map::find(const char *key, const AST::Element **scalar, AST::Element::Type type) const {
  if (!key || !scalar) {
    return ESB_NULL_POINTER;
  }

  const AST::Element *e = find(key);

  if (!e) {
    return ESB_MISSING_FIELD;
  }

  if (type != e->type()) {
    return ESB_INVALID_FIELD;
  }

  *scalar = e;
  return ESB_SUCCESS;
}

Error Map::find(const char *key, const AST::Integer **integer, Int64 min, Int64 max) const {
  const AST::Integer *tmp = NULL;
  Error error = find(key, (const AST::Element **)&tmp, AST::Element::INTEGER);
  if (ESB_SUCCESS != error) {
    return error;
  }

  if (min > tmp->value() || max < tmp->value()) {
    return ESB_INVALID_FIELD;
  }

  *integer = tmp;
  return ESB_SUCCESS;
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
