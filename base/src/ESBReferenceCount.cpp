#ifndef ESB_REFERENCE_COUNT_H
#include <ESBReferenceCount.h>
#endif

namespace ESB {

ReferenceCount::ReferenceCount() : _refCount() {
  //
  // Important: we are purposely not initializing the _allocator attribute
  // because that would overwrite the pointer assigned by operator new.
  //
}

ReferenceCount::~ReferenceCount() { assert(0 == _refCount.get()); }

}  // namespace ESB
