#ifndef ESB_JSON_MAP_H
#include <ESBJsonMap.h>
#endif

#ifndef ESB_JSON_INTEGER_H
#include <ESBJsonInteger.h>
#endif

#ifndef ESB_JSON_DECIMAL_H
#include <ESBJsonDecimal.h>
#endif

#ifndef ESB_JSON_STRING_H
#include <ESBJsonString.h>
#endif

namespace ESB {

JsonMap::JsonMapComparator JsonMap::_Comparator;

JsonMap::JsonMap(Allocator &allocator) : JsonElement(), _map(_Comparator, NullLock::Instance(), allocator) {}

JsonMap::~JsonMap() { clear(); }

JsonElement::Type JsonMap::type() const { return JsonElement::MAP; }

JsonMapIterator JsonMap::iterator() {
  JsonMapIterator it(_map.minimumIterator());
  return it;
}

Error JsonMap::clear() {
  for (JsonMapIterator it = iterator(); !it.isNull(); ++it) {
    JsonScalar *key = (JsonScalar *)it.key();
    JsonElement *value = (JsonElement *)it.value();

    if (key && key->cleanupHandler()) {
      key->cleanupHandler()->destroy(key);
    }

    if (value && value->cleanupHandler()) {
      value->cleanupHandler()->destroy(value);
    }
  }

  return _map.clear();
}

int JsonMap::JsonMapComparator::compare(const void *l, const void *r) const {
  const JsonScalar *left = (const JsonScalar *)l;
  const JsonScalar *right = (const JsonScalar *)r;

  if (left->type() != right->type()) {
    return left->type() - right->type();
  }

  switch (left->type()) {
    case JsonElement::STRING:
      return ((const JsonString *)left)->compare(*((const JsonString *)right));
    case JsonElement::INTEGER:
      return ((const JsonInteger *)left)->compare(*((const JsonInteger *)right));
    case JsonElement::DECIMAL:
      return ((const JsonDecimal *)left)->compare(*((const JsonDecimal *)right));
    default:
      assert(!"Invalid json scalar type");
      return -1;
  }
}

JsonMapIterator::JsonMapIterator() : _it() {}

JsonMapIterator::JsonMapIterator(const JsonMapIterator &it) : _it(it._it) {}

JsonMapIterator::~JsonMapIterator() {}

JsonMapIterator::JsonMapIterator(const MapIterator &it) : _it(it) {}

}  // namespace ESB
