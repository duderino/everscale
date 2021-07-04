#ifndef ESB_JSON_SCALAR_H
#include <ESBJsonScalar.h>
#endif

namespace ESB {

JsonScalar::JsonScalar(Allocator &allocator) : JsonElement(allocator) {}

JsonScalar::~JsonScalar() {}

}  // namespace ESB
