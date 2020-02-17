#ifndef ESB_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESBAllocatorCleanupHandler.h>
#endif

namespace ESB {

AllocatorCleanupHandler::AllocatorCleanupHandler(Allocator *allocator)
    : _allocator(allocator) {
  assert(_allocator);
}

AllocatorCleanupHandler::~AllocatorCleanupHandler() {}

void AllocatorCleanupHandler::destroy(Object *object) {
  if (!_allocator || !object) {
    return;
  }

  object->~Object();
  _allocator->deallocate(object);
}

}  // namespace ESB
