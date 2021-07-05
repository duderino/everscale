#ifndef ESB_AST_STRING_H
#include <ASTString.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

namespace ESB {
namespace AST {

String::String(Allocator &allocator) : Scalar(allocator), _value(NULL) {}

String::~String() {
  if (_value) {
    _allocator.deallocate(_value);
    _value = NULL;
  }
}

Error String::setValue(const char *buffer, UWord size) {
  char *value = NULL;
  Error error = Duplicate(buffer, size, _allocator, &value);
  if (ESB_SUCCESS != error) {
    return error;
  }

  if (_value) {
    _allocator.deallocate(_value);
  }

  _value = value;
  return ESB_SUCCESS;
}

int String::compare(const String &other) const {
  if (_value == other._value) {
    return 0;
  }

  if (!_value) {
    return -1;
  }

  if (!other._value) {
    return 1;
  }

  return strcmp(_value, other._value);
}

Element::Type String::type() const { return STRING; }

}  // namespace AST
}  // namespace ESB
