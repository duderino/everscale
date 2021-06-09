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

Error SystemAllocator::allocate(UWord size, void **block) {
  if (0 == size) {
    return ESB_INVALID_ARGUMENT;
  }

  if (!block) {
    return ESB_NULL_POINTER;
  }

#ifdef HAVE_MALLOC
  void *mem = malloc(size);
  if (mem) {
    *block = mem;
    return ESB_SUCCESS;
  } else {
    return ESB_OUT_OF_MEMORY;
  }
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

Error SystemAllocator::reallocate(void *oldBlock, UWord size, void **newBlock) {
  if (!oldBlock && 0 == size) {
    return ESB_INVALID_ARGUMENT;
  }

#ifdef HAVE_REALLOC
  *newBlock = realloc(oldBlock, size);
  return *newBlock ? ESB_SUCCESS : ESB_OUT_OF_MEMORY;
#else
#error "Platform requires realloc or equivalent"
#endif
}

}  // namespace ESB
