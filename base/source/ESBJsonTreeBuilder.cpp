#ifndef ESB_JSON_TREE_BUILDER_H
#include <ESBJsonTreeBuilder.h>
#endif

namespace ESB {

JsonTreeBuilder::JsonTreeBuilder(Allocator &allocator)
    : _stack(), _error(ESB_SUCCESS), _root(NULL), _allocator(allocator) {}

JsonTreeBuilder::~JsonTreeBuilder() {
  if (_root && _root->cleanupHandler()) {
    _root->cleanupHandler()->destroy(_root);
  }

  while (true) {
    JsonElement *e = (JsonElement *)_stack.removeLast();
    if (!e) {
      break;
    }
    if (e->cleanupHandler()) {
      e->cleanupHandler()->destroy(e);
    }
  }
}

JsonCallbacks::ParseControl JsonTreeBuilder::onMapStart() {
  JsonMap *map = new (_allocator) JsonMap(_allocator);

  if (!map) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  _stack.addLast(map);
  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onMapKey(const unsigned char *key, UInt32 length) {
  JsonString *newString = new (_allocator) JsonString(_allocator);

  if (!newString) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  Error error = newString->setValue((const char *)key, length);
  if (ESB_SUCCESS != error) {
    _error = error;
    newString->~JsonString();
    _allocator.deallocate(newString);
    return BREAK;
  }

  _stack.addLast(newString);
  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onMapEnd() {
  JsonElement *newMap = (JsonElement *)_stack.removeLast();

  assert(JsonElement::MAP == newMap->type());
  if (JsonElement::MAP != newMap->type()) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  JsonElement *top = (JsonElement *)_stack.last();

  if (!top) {
    assert(!_root);
    _root = newMap;
    return CONTINUE;
  }

  if (JsonElement::LIST == top->type()) {
    JsonList *list = (JsonList *)top;
    list->addLast(newMap);
    return CONTINUE;
  }

  if (BREAK == insertKeyPair(newMap)) {
    _stack.addLast(newMap);
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onListStart() {
  JsonList *list = new (_allocator) JsonList(_allocator);

  if (!list) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  _stack.addLast(list);
  return CONTINUE;
}
JsonCallbacks::ParseControl JsonTreeBuilder::onListEnd() {
  JsonElement *newList = (JsonElement *)_stack.removeLast();

  assert(JsonElement::LIST == newList->type());
  if (JsonElement::LIST != newList->type()) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  JsonElement *top = (JsonElement *)_stack.last();

  if (!top) {
    assert(!_root);
    _root = newList;
    return CONTINUE;
  }

  if (JsonElement::LIST == top->type()) {
    JsonList *list = (JsonList *)top;
    list->addLast(newList);
    return CONTINUE;
  }

  if (BREAK == insertKeyPair(newList)) {
    _stack.addLast(newList);
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onNull() {
  JsonElement *top = (JsonElement *)_stack.last();
  assert(top);
  if (!top) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  switch (top->type()) {
    case JsonElement::LIST:
    case JsonElement::BOOLEAN:
    case JsonElement::STRING:
    case JsonElement::INTEGER:
    case JsonElement::DECIMAL:
      break;
    default:
      _error = ESB_CANNOT_PARSE;
      return BREAK;
  }

  JsonNull *newNull = new (_allocator) JsonNull(_allocator);
  if (!newNull) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (JsonElement::LIST == top->type()) {
    JsonList *list = (JsonList *)top;
    list->addLast(newNull);
    return CONTINUE;
  }

  if (BREAK == insertKeyPair(newNull)) {
    newNull->~JsonNull();
    _allocator.deallocate(newNull);
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onBoolean(bool value) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  JsonBoolean *newBoolean = new (_allocator) JsonBoolean(value, _allocator);
  if (!newBoolean) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newBoolean)) {
    newBoolean->~JsonBoolean();
    _allocator.deallocate(newBoolean);
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onInteger(Int64 value) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  JsonInteger *newInteger = new (_allocator) JsonInteger(value, _allocator);
  if (!newInteger) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newInteger)) {
    newInteger->~JsonInteger();
    _allocator.deallocate(newInteger);
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onDouble(double value) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  JsonDecimal *newDecimal = new (_allocator) JsonDecimal(value, _allocator);
  if (!newDecimal) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newDecimal)) {
    newDecimal->~JsonDecimal();
    _allocator.deallocate(newDecimal);
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::onString(const unsigned char *value, UInt32 length) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  JsonString *newString = new (_allocator) JsonString(_allocator);
  if (!newString) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  Error error = newString->setValue((const char *)value, length);
  if (ESB_SUCCESS != error) {
    _error = error;
    newString->~JsonString();
    _allocator.deallocate(newString);
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newString)) {
    newString->~JsonString();
    _allocator.deallocate(newString);
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::validateKeyType() {
  JsonElement *top = (JsonElement *)_stack.last();
  assert(top);
  if (!top) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  switch (top->type()) {
    case JsonElement::LIST:
    case JsonElement::BOOLEAN:
    case JsonElement::STRING:
    case JsonElement::INTEGER:
    case JsonElement::DECIMAL:
    case JsonElement::MAP:
      return CONTINUE;
    default:
      _error = ESB_CANNOT_PARSE;
      return BREAK;
  }
}

JsonCallbacks::ParseControl JsonTreeBuilder::insertKeyOrValue(JsonElement *element) {
  JsonElement *top = (JsonElement *)_stack.last();

  if (JsonElement::LIST == top->type()) {
    JsonList *list = (JsonList *)top;
    list->addLast(element);
    return CONTINUE;
  }

  if (JsonElement::MAP == top->type()) {
    _stack.addLast(element);
    return CONTINUE;
  }

  return insertKeyPair(element);
}

JsonCallbacks::ParseControl JsonTreeBuilder::insertKeyPair(JsonElement *element) {
  JsonElement *top = (JsonElement *)_stack.last();

  switch (top->type()) {
    case JsonElement::BOOLEAN:
    case JsonElement::STRING:
    case JsonElement::INTEGER:
    case JsonElement::DECIMAL: {
      JsonScalar *key = (JsonScalar *)_stack.removeLast();
      top = (JsonElement *)_stack.last();

      assert(JsonElement::MAP == top->type());
      if (JsonElement::MAP != top->type()) {
        _error = ESB_CANNOT_PARSE;
        return BREAK;
      }

      JsonMap *map = (JsonMap *)_stack.last();
      Error error = map->insert(key, element);
      if (ESB_SUCCESS != error) {
        _error = error;
        return BREAK;
      }

      return CONTINUE;
    }
    default:
      assert(!"Top of stack must be a scalar key");
      _error = ESB_CANNOT_PARSE;
      return BREAK;
  }
}

void JsonTreeBuilder::traverse(JsonCallbacks &callbacks) const {
  if (_root) {
    traverseElement(callbacks, _root);
  }
}

JsonCallbacks::ParseControl JsonTreeBuilder::traverseMap(JsonCallbacks &callbacks, JsonMap *map) const {
  if (BREAK == callbacks.onMapStart()) {
    return BREAK;
  }

  for (JsonMapIterator it = map->iterator(); !it.isNull(); ++it) {
    // yajl only supports string keys
    assert(JsonElement::STRING == it.key()->type());
    JsonString *key = (JsonString *)it.key();
    if (BREAK == callbacks.onMapKey((const unsigned char *)key->value(), strlen(key->value()))) {
      return BREAK;
    }
    if (BREAK == traverseElement(callbacks, it.value())) {
      return BREAK;
    }
  }

  if (BREAK == callbacks.onMapEnd()) {
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::traverseList(JsonCallbacks &callbacks, JsonList *list) const {
  if (BREAK == callbacks.onListStart()) {
    return BREAK;
  }

  for (JsonElement *element = list->first(); element; element = (JsonElement *)element->next()) {
    if (BREAK == traverseElement(callbacks, element)) {
      return BREAK;
    }
  }

  if (BREAK == callbacks.onListEnd()) {
    return BREAK;
  }

  return CONTINUE;
}

JsonCallbacks::ParseControl JsonTreeBuilder::traverseElement(JsonCallbacks &callbacks, JsonElement *element) const {
  switch (element->type()) {
    case JsonElement::MAP:
      if (BREAK == traverseMap(callbacks, (JsonMap *)element)) {
        return BREAK;
      }
      break;
    case JsonElement::LIST:
      if (BREAK == traverseList(callbacks, (JsonList *)element)) {
        return BREAK;
      }
      break;
    case JsonElement::NIL:
      if (BREAK == callbacks.onNull()) {
        return BREAK;
      }
      break;
    case JsonElement::STRING: {
      JsonString *value = (JsonString *)element;
      if (BREAK == callbacks.onString((const unsigned char *)value->value(), strlen(value->value()))) {
        return BREAK;
      }
      break;
    }
    case JsonElement::INTEGER: {
      JsonInteger *value = (JsonInteger *)element;
      if (BREAK == callbacks.onInteger(value->value())) {
        return BREAK;
      }
      break;
    }
    case JsonElement::DECIMAL: {
      JsonDecimal *value = (JsonDecimal *)element;
      if (BREAK == callbacks.onDouble(value->value())) {
        return BREAK;
      }
      break;
    }
    case JsonElement::BOOLEAN: {
      JsonBoolean *value = (JsonBoolean *)element;
      if (BREAK == callbacks.onBoolean(value->value())) {
        return BREAK;
      }
      break;
    }
  }

  return CONTINUE;
}

}  // namespace ESB
