#ifndef ESB_JSON_STRING_H
#include <ESBJsonString.h>
#endif

#ifndef ESB_STRING_H
#include <ESBString.h>
#endif

namespace ESB {

JsonString::JsonString(Allocator &allocator) : JsonScalar(allocator), _value(NULL) {}

JsonString::~JsonString() {
  if (_value) {
    _allocator.deallocate(_value);
    _value = NULL;
  }
}

Error JsonString::setValue(const char *buffer, UWord size) {
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

int JsonString::compare(const JsonString &other) const {
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

JsonElement::Type JsonString::type() const { return STRING; }

}  // namespace ESB
