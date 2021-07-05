#ifndef ESB_AST_TREE_BUILDER_H
#define ESB_AST_TREE_BUILDER_H

#ifndef ESB_AST_MAP_H
#include <ASTMap.h>
#endif

#ifndef ESB_AST_STRING_H
#include <ASTString.h>
#endif

#ifndef ESB_AST_LIST_H
#include <ASTList.h>
#endif

#ifndef ESB_AST_NULL_H
#include <ASTNull.h>
#endif

#ifndef ESB_AST_INTEGER_H
#include <ASTInteger.h>
#endif

#ifndef ESB_AST_DECIMAL_H
#include <ASTDecimal.h>
#endif

#ifndef ESB_AST_BOOLEAN_H
#include <ASTBoolean.h>
#endif

#ifndef ESB_AST_CALLBACKS_H
#include <ASTCallbacks.h>
#endif

#ifndef ESB_EMBEDDED_LIST_H
#include <ESBEmbeddedList.h>
#endif

namespace ESB {
namespace AST {

/**
 *  An AST tree builder
 *
 *  @ingroup ast
 */
class Tree : public Callbacks {
 public:
  Tree(Allocator &allocator = SystemAllocator::Instance());

  /** Destructor. */
  virtual ~Tree();

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

  inline Element *root() { return _root; }

  inline const Element *root() const { return _root; }

  /**
   * Traverse the root of the JSON tree
   *
   * @param callbacks A set of callbacks
   */
  void traverse(Callbacks &callbacks) const;

 private:
  ParseControl traverseElement(Callbacks &callbacks, Element *element) const;
  ParseControl traverseMap(Callbacks &callbacks, Map *map) const;
  ParseControl traverseList(Callbacks &callbacks, List *list) const;
  ParseControl validateKeyType();
  ParseControl insertKeyOrValue(Element *element);
  ParseControl insertKeyPair(Element *element);

  //
  // The type of the element at the top of the stack is the current state.  When parsing a new entity, check the type at
  // the top of the stack.  If...
  //
  // List type: if scalar, add the new entity to the end of the list, else push new container onto the stack Map type:
  // push the new entity into the stack, this will become a key.  Entity must be a scalar. Scalar type: Pop the scalar
  // from the top of the stack.  Insert the popped scalar as the key + new entity as the value into the map which should
  // now be at the top of the stack.
  //
  EmbeddedList _stack;
  Error _error;
  Element *_root;
  Allocator &_allocator;

  ESB_DEFAULT_FUNCS(Tree);
};

}  // namespace AST
}  // namespace ESB

#endif
