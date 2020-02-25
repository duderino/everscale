#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ESB {

SystemAllocator SystemAllocator::_Allocator;

SystemAllocator *SystemAllocator::GetInstance() { return &_Allocator; }

SystemAllocator::SystemAllocator() {}

SystemAllocator::~SystemAllocator() {}

void *SystemAllocator::allocate(UWord size) {
  if (0 == size) {
    return 0;
  }

#ifdef HAVE_MALLOC
  return malloc(size);
#else
#error "Platform requires malloc or equivalent"
#endif
}

Error SystemAllocator::allocate(void **block, UWord size) {
  if (!block) {
    return ESB_NULL_POINTER;
  }

  if (0 == size) {
    return ESB_INVALID_ARGUMENT;
  }

  void *tmp = 0;

#ifdef HAVE_MALLOC
  tmp = malloc(size);
#else
#error "Platform requires malloc or equivalent"
#endif

  if (!tmp) {
    return ESB_OUT_OF_MEMORY;
  }

  *block = tmp;

  return ESB_SUCCESS;
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

UWord SystemAllocator::getOverhead() { return ESB_UWORD_C(0); }

Error SystemAllocator::initialize() { return ESB_SUCCESS; }

Error SystemAllocator::destroy() { return ESB_OPERATION_NOT_SUPPORTED; }

Error SystemAllocator::isInitialized() { return ESB_SUCCESS; }

Error SystemAllocator::setFailoverAllocator(Allocator *allocator) {
  return ESB_OPERATION_NOT_SUPPORTED;
}

Error SystemAllocator::getFailoverAllocator(Allocator **allocator) {
  return ESB_OPERATION_NOT_SUPPORTED;
}

}  // namespace ESB
