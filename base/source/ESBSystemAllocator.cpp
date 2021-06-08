#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ESB {

SystemAllocator SystemAllocator::_Allocator;

SystemAllocator &SystemAllocator::Instance() { return _Allocator; }

SystemAllocator::SystemAllocator() : _cleanupHandler(*this) {}

SystemAllocator::~SystemAllocator() {}

void *SystemAllocator::allocate(UWord size) {
  if (0 == size) {
    return NULL;
  }

#ifdef HAVE_MALLOC
  return malloc(size);
#else
#error "Platform requires malloc or equivalent"
#endif
}

Error SystemAllocator::deallocate(void *block) {
  if (!block) {
    return ESB_NULL_POINTER;
  }

#ifdef HAVE_FREE
  free(block);
#else
#error "Platform requires free or equivalent"
#endif

  return ESB_SUCCESS;
}

CleanupHandler &SystemAllocator::cleanupHandler() { return _cleanupHandler; }

bool SystemAllocator::reallocates() { return true; }

void *SystemAllocator::reallocate(void *block, UWord size) {
  if (!block && 0 == size) {
    return NULL;
  }

#ifdef HAVE_REALLOC
  return realloc(block, size);
#else
#error "Platform requires realloc or equivalent"
#endif
}

}  // namespace ESB
