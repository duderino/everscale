#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

Allocator::Allocator() {}

Allocator::~Allocator() {}

AllocatorCleanupHandler::AllocatorCleanupHandler(ESB::Allocator &allocator) : _allocator(allocator) {}

AllocatorCleanupHandler::~AllocatorCleanupHandler() {}

void AllocatorCleanupHandler::destroy(ESB::Object *object) {
  if (!object) {
    return;
  }

  object->~Object();
  _allocator.deallocate(object);
}

}  // namespace ESB
