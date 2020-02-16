/** @file ESBAllocatorCleanupHandler.cpp
 *  @brief An object that can destroy another object created by an allocator
 */

#ifndef ESB_ALLOCATOR_CLEANUP_HANDLER_H
#include <ESBAllocatorCleanupHandler.h>
#endif

#ifndef ESB_ASSERT_H
#include <ESBAssert.h>
#endif

namespace ESB {

AllocatorCleanupHandler::AllocatorCleanupHandler(Allocator *allocator)
    : _allocator(allocator) {
  ESB_ASSERT(_allocator);
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
