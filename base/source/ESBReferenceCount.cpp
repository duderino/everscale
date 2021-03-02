#ifndef ESB_REFERENCE_COUNT_H
#include <ESBReferenceCount.h>
#endif

namespace ESB {

ReferenceCount::ReferenceCount() : _refCount(0) {}

ReferenceCount::~ReferenceCount() { assert(0 == _refCount.get()); }

}  // namespace ESB
