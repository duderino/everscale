#ifndef ESB_JSON_TREE_BUILDER_H
#define ESB_JSON_TREE_BUILDER_H

#ifndef ESB_JSON_MAP_H
#include <ESBJsonMap.h>
#endif

#ifndef ESB_JSON_STRING_H
#include <ESBJsonString.h>
#endif

#ifndef ESB_JSON_LIST_H
#include <ESBJsonList.h>
#endif

#ifndef ESB_JSON_NULL_H
#include <ESBJsonNull.h>
#endif

#ifndef ESB_JSON_INTEGER_H
#include <ESBJsonInteger.h>
#endif

#ifndef ESB_JSON_DECIMAL_H
#include <ESBJsonDecimal.h>
#endif

#ifndef ESB_JSON_BOOLEAN_H
#include <ESBJsonBoolean.h>
#endif

#ifndef ESB_JSON_CALLBACKS_H
#include <ESBJsonCallbacks.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ESB {

/**
 *  A JSON tree builder, pass this to the JsonParser's constructor.
 *
 *  @ingroup json
 */
class JsonTreeBuilder : public JsonCallbacks {
 public:
  JsonTreeBuilder(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor. */
  virtual ~JsonTreeBuilder();

  virtual ParseControl onMapStart();
  virtual ParseControl onMapKey(const unsigned char *key, UInt32 length);
  virtual ParseControl onMapEnd();
  virtual ParseControl onListStart();
  virtual ParseControl onListEnd();
  virtual ParseControl onNull();
  virtual ParseControl onBoolean(bool value);
  virtual ParseControl onInteger(Int64 value);
  virtual ParseControl onDouble(double value);
  virtual ParseControl onString(const unsigned char *value, UInt32 length);

  inline Error result() const { return _error; }

  inline JsonElement *root() { return _root; }

  inline const JsonElement *root() const { return _root; }

  /**
   * Traverse the root of the JSON tree
   *
   * @param callbacks A set of callbacks
   */
  void traverse(JsonCallbacks &callbacks) const;

 private:
  ParseControl traverseElement(JsonCallbacks &callbacks, JsonElement *element) const;
  ParseControl traverseMap(JsonCallbacks &callbacks, JsonMap *map) const;
  ParseControl traverseList(JsonCallbacks &callbacks, JsonList *list) const;
  ParseControl validateKeyType();
  ParseControl insertKeyOrValue(JsonElement *element);
  ParseControl insertKeyPair(JsonElement *element);

  //
  // The type of the element at the top of the stack is the current state.  When parsing a new entity, check the type at
  // the top of the stack.  If...
  //
  // List type: if scalar, add the new entity to the end of the list, else push new container onto the stack
  // Map type: push the new entity into the stack, this will become a key.  Entity must be a scalar.
  // Scalar type: Pop the scalar from the top of the stack.  Insert the popped scalar as the key + new entity as the
  // value into the map which should now be at the top of the stack.
  //
  EmbeddedList _stack;
  Error _error;
  JsonElement *_root;
  Allocator &_allocator;

  ESB_DEFAULT_FUNCS(JsonTreeBuilder);
};

}  // namespace ESB

#endif
