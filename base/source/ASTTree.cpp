#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

namespace ESB {
namespace AST {

Tree::Tree(Allocator &allocator) : _stack(), _error(ESB_SUCCESS), _root(NULL), _allocator(allocator) {}

Tree::~Tree() {
  if (_root && _root->cleanupHandler()) {
    _root->cleanupHandler()->destroy(_root);
  }

  while (true) {
    Element *e = (Element *)_stack.removeLast();
    if (!e) {
      break;
    }
    if (e->cleanupHandler()) {
      e->cleanupHandler()->destroy(e);
    }
  }
}

Callbacks::ParseControl Tree::onMapStart() {
  Map *map = new (_allocator) Map(_allocator);

  if (!map) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  _stack.addLast(map);
  return CONTINUE;
}

Callbacks::ParseControl Tree::onMapKey(const unsigned char *key, UInt32 length) {
  String *newString = new (_allocator) String(_allocator);

  if (!newString) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  Error error = newString->setValue((const char *)key, length);
  if (ESB_SUCCESS != error) {
    _error = error;
    newString->~String();
    _allocator.deallocate(newString);
    return BREAK;
  }

  _stack.addLast(newString);
  return CONTINUE;
}

Callbacks::ParseControl Tree::onMapEnd() {
  Element *newMap = (Element *)_stack.removeLast();

  assert(Element::MAP == newMap->type());
  if (Element::MAP != newMap->type()) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  Element *top = (Element *)_stack.last();

  if (!top) {
    assert(!_root);
    _root = newMap;
    return CONTINUE;
  }

  if (Element::LIST == top->type()) {
    List *list = (List *)top;
    list->addLast(newMap);
    return CONTINUE;
  }

  if (BREAK == insertKeyPair(newMap)) {
    _stack.addLast(newMap);
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::onListStart() {
  List *list = new (_allocator) List(_allocator);

  if (!list) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  _stack.addLast(list);
  return CONTINUE;
}

Callbacks::ParseControl Tree::onListEnd() {
  Element *newList = (Element *)_stack.removeLast();

  assert(Element::LIST == newList->type());
  if (Element::LIST != newList->type()) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  Element *top = (Element *)_stack.last();

  if (!top) {
    assert(!_root);
    _root = newList;
    return CONTINUE;
  }

  if (Element::LIST == top->type()) {
    List *list = (List *)top;
    list->addLast(newList);
    return CONTINUE;
  }

  if (BREAK == insertKeyPair(newList)) {
    _stack.addLast(newList);
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::onNull() {
  Element *top = (Element *)_stack.last();
  assert(top);
  if (!top) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  switch (top->type()) {
    case Element::LIST:
    case Element::BOOLEAN:
    case Element::STRING:
    case Element::INTEGER:
    case Element::DECIMAL:
      break;
    default:
      _error = ESB_CANNOT_PARSE;
      return BREAK;
  }

  Null *newNull = new (_allocator) Null(_allocator);
  if (!newNull) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (Element::LIST == top->type()) {
    List *list = (List *)top;
    list->addLast(newNull);
    return CONTINUE;
  }

  if (BREAK == insertKeyPair(newNull)) {
    newNull->~Null();
    _allocator.deallocate(newNull);
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::onBoolean(bool value) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  Boolean *newBoolean = new (_allocator) Boolean(value, _allocator);
  if (!newBoolean) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newBoolean)) {
    newBoolean->~Boolean();
    _allocator.deallocate(newBoolean);
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::onInteger(Int64 value) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  Integer *newInteger = new (_allocator) Integer(value, _allocator);
  if (!newInteger) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newInteger)) {
    newInteger->~Integer();
    _allocator.deallocate(newInteger);
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::onDouble(double value) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  Decimal *newDecimal = new (_allocator) Decimal(value, _allocator);
  if (!newDecimal) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newDecimal)) {
    newDecimal->~Decimal();
    _allocator.deallocate(newDecimal);
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::onString(const unsigned char *value, UInt32 length) {
  if (BREAK == validateKeyType()) {
    return BREAK;
  }

  String *newString = new (_allocator) String(_allocator);
  if (!newString) {
    _error = ESB_OUT_OF_MEMORY;
    return BREAK;
  }

  Error error = newString->setValue((const char *)value, length);
  if (ESB_SUCCESS != error) {
    _error = error;
    newString->~String();
    _allocator.deallocate(newString);
    return BREAK;
  }

  if (BREAK == insertKeyOrValue(newString)) {
    newString->~String();
    _allocator.deallocate(newString);
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::validateKeyType() {
  Element *top = (Element *)_stack.last();
  assert(top);
  if (!top) {
    _error = ESB_CANNOT_PARSE;
    return BREAK;
  }

  switch (top->type()) {
    case Element::LIST:
    case Element::BOOLEAN:
    case Element::STRING:
    case Element::INTEGER:
    case Element::DECIMAL:
    case Element::MAP:
      return CONTINUE;
    default:
      _error = ESB_CANNOT_PARSE;
      return BREAK;
  }
}

Callbacks::ParseControl Tree::insertKeyOrValue(Element *element) {
  Element *top = (Element *)_stack.last();

  if (Element::LIST == top->type()) {
    List *list = (List *)top;
    list->addLast(element);
    return CONTINUE;
  }

  if (Element::MAP == top->type()) {
    _stack.addLast(element);
    return CONTINUE;
  }

  return insertKeyPair(element);
}

Callbacks::ParseControl Tree::insertKeyPair(Element *element) {
  Element *top = (Element *)_stack.last();

  switch (top->type()) {
    case Element::BOOLEAN:
    case Element::STRING:
    case Element::INTEGER:
    case Element::DECIMAL: {
      Scalar *key = (Scalar *)_stack.removeLast();
      top = (Element *)_stack.last();

      assert(Element::MAP == top->type());
      if (Element::MAP != top->type()) {
        _error = ESB_CANNOT_PARSE;
        return BREAK;
      }

      Map *map = (Map *)_stack.last();
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

void Tree::traverse(Callbacks &callbacks) const {
  if (_root) {
    traverseElement(callbacks, _root);
  }
}

Callbacks::ParseControl Tree::traverseMap(Callbacks &callbacks, Map *map) const {
  if (BREAK == callbacks.onMapStart()) {
    return BREAK;
  }

  for (MapIterator it = map->iterator(); !it.isNull(); ++it) {
    // yajl only supports string keys
    assert(Element::STRING == it.key()->type());
    String *key = (String *)it.key();
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

Callbacks::ParseControl Tree::traverseList(Callbacks &callbacks, List *list) const {
  if (BREAK == callbacks.onListStart()) {
    return BREAK;
  }

  for (Element *element = list->first(); element; element = (Element *)element->next()) {
    if (BREAK == traverseElement(callbacks, element)) {
      return BREAK;
    }
  }

  if (BREAK == callbacks.onListEnd()) {
    return BREAK;
  }

  return CONTINUE;
}

Callbacks::ParseControl Tree::traverseElement(Callbacks &callbacks, Element *element) const {
  switch (element->type()) {
    case Element::MAP:
      if (BREAK == traverseMap(callbacks, (Map *)element)) {
        return BREAK;
      }
      break;
    case Element::LIST:
      if (BREAK == traverseList(callbacks, (List *)element)) {
        return BREAK;
      }
      break;
    case Element::NIL:
      if (BREAK == callbacks.onNull()) {
        return BREAK;
      }
      break;
    case Element::STRING: {
      String *value = (String *)element;
      if (BREAK == callbacks.onString((const unsigned char *)value->value(), strlen(value->value()))) {
        return BREAK;
      }
      break;
    }
    case Element::INTEGER: {
      Integer *value = (Integer *)element;
      if (BREAK == callbacks.onInteger(value->value())) {
        return BREAK;
      }
      break;
    }
    case Element::DECIMAL: {
      Decimal *value = (Decimal *)element;
      if (BREAK == callbacks.onDouble(value->value())) {
        return BREAK;
      }
      break;
    }
    case Element::BOOLEAN: {
      Boolean *value = (Boolean *)element;
      if (BREAK == callbacks.onBoolean(value->value())) {
        return BREAK;
      }
      break;
    }
  }

  return CONTINUE;
}

}  // namespace AST
}  // namespace ESB
