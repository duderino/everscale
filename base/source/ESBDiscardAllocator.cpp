#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

namespace ESB {

DiscardAllocator::DiscardAllocator(UInt32 chunkSize, UInt16 alignmentSize, UInt16 multipleOf, Allocator &source,
                                   bool forcePool)
    : _head(NULL),
#ifdef ESB_NO_ALLOC
      _forcePool(forcePool),
#endif
      _alignmentSize(alignmentSize),
      _multipleOf(multipleOf),
      _chunkSize(ESB_ALIGN(128 > chunkSize ? 128 : chunkSize, _alignmentSize)),
      _source(source),
      _cleanupHandler(*this) {
}

DiscardAllocator::~DiscardAllocator() {
  Chunk *current = _head;
  Chunk *next = NULL;

  while (current) {
    next = current->_next;
    _source.deallocate(current);
    current = next;
  }

  _head = NULL;
}

Error DiscardAllocator::allocate(UWord size, void **block) {
#ifdef ESB_NO_ALLOC
  if (!_forcePool) {
    return SystemAllocator::Instance().allocate(size, block);
  }
#endif
  assert(0 < size);
  if (0 == size) {
    return ESB_INVALID_ARGUMENT;
  }

  if (!block) {
    return ESB_NULL_POINTER;
  }

  //
  // If asked for a block of memory larger than the chunk size, bypass
  // the chunks and allocate memory directly from the source allocator,
  // but remember the allocated memory so it can be freed when this
  // allocator is destroyed.
  //

  if (size > _chunkSize) {
    Chunk *chunk = NULL;
    Error error = allocateChunk(size, &chunk);
    if (ESB_SUCCESS != error) {
      return error;
    }

    chunk->_idx = chunk->_size;

    if (_head) {
      chunk->_next = _head->_next;
      _head->_next = chunk;
    } else {
      _head = chunk;
    }

    *block = chunk->_data;
    return ESB_SUCCESS;
  }

  if (!_head) {
    Error error = allocateChunk(_chunkSize, &_head);
    if (ESB_SUCCESS != error) {
      return error;
    }
  }

  if (size > (_head->_idx > _head->_size ? 0 : _head->_size - _head->_idx)) {
    Chunk *newHead = NULL;
    Error error = allocateChunk(_chunkSize, &newHead);
    if (ESB_SUCCESS != error) {
      return error;
    }

    newHead->_next = _head;
    _head = newHead;
  }

  *block = _head->_data + _head->_idx;
  // Always keep the next available block aligned
  _head->_idx += ESB_ALIGN(size, _alignmentSize);
  return ESB_SUCCESS;
}

Error DiscardAllocator::deallocate(void *block) {
#ifdef ESB_NO_ALLOC
  if (!_forcePool) {
    return SystemAllocator::Instance().deallocate(block);
  }
#endif
  return block ? ESB_SUCCESS : ESB_NULL_POINTER;
}

Error DiscardAllocator::reset() {
  Chunk *current = _head;
  Chunk *next = NULL;
  _head = NULL;

  while (current && current->_next) {
    next = current->_next;
    _source.deallocate(current);
    current = next;
  }

  _head = current;

  if (_head) {
    _head->_idx = 0;
  }

  return ESB_SUCCESS;
}

Error DiscardAllocator::allocateChunk(int chunkSize, Chunk **chunk) {
  ESB::UInt32 size = SizeofChunk(_alignmentSize) + chunkSize;
  assert(0 == size % _multipleOf);
  if (0 != size % _multipleOf) {
    return ESB_INVALID_STATE;
  }

  Error error = _source.allocate(size, (void **)chunk);
  if (ESB_SUCCESS != error) {
    return error;
  }

  (*chunk)->_next = NULL;
  (*chunk)->_idx = 0;
  (*chunk)->_size = chunkSize;

  // Always keep the next available block aligned
  (*chunk)->_data = ((char *)*chunk) + SizeofChunk(_alignmentSize);

  return ESB_SUCCESS;
}

CleanupHandler &DiscardAllocator::cleanupHandler() { return _cleanupHandler; }

bool DiscardAllocator::reallocates() { return false; }

Error DiscardAllocator::reallocate(void *oldBlock, UWord size, void **newBlock) { return ESB_OPERATION_NOT_SUPPORTED; }

}  // namespace ESB
