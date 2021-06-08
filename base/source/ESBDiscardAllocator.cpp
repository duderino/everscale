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

void *DiscardAllocator::allocate(UWord size) {
#ifdef ESB_NO_ALLOC
  if (!_forcePool) {
    return SystemAllocator::Instance().allocate(size);
  }
#endif
  assert(0 < size);

  if (1 > size) {
    return NULL;
  }

  //
  // If asked for a block of memory larger than the chunk size, bypass
  // the chunks and allocate memory directly from the source allocator,
  // but remember the allocated memory so it can be freed when this
  // allocator is destroyed.
  //

  if (size > _chunkSize) {
    Chunk *chunk = allocateChunk(size);

    if (!chunk) {
      return NULL;
    }

    chunk->_idx = chunk->_size;

    if (_head) {
      chunk->_next = _head->_next;
      _head->_next = chunk;
    } else {
      _head = chunk;
    }

    return chunk->_data;
  }

  if (!_head) {
    _head = allocateChunk(_chunkSize);
    if (!_head) {
      return NULL;
    }
  }

  if (size > (_head->_idx > _head->_size ? 0 : _head->_size - _head->_idx)) {
    Chunk *oldHead = _head;

    _head = allocateChunk(_chunkSize);

    if (!_head) {
      _head = oldHead;
      return NULL;
    }

    _head->_next = oldHead;
  }

  void *block = _head->_data + _head->_idx;

  // Always keep the next available block aligned
  _head->_idx += ESB_ALIGN(size, _alignmentSize);

  return block;
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

DiscardAllocator::Chunk *DiscardAllocator::allocateChunk(int chunkSize) {
  ESB::UInt32 size = SizeofChunk(_alignmentSize) + chunkSize;
  assert(0 == size % _multipleOf);
  if (0 != size % _multipleOf) {
    return NULL;
  }

  Chunk *chunk = (Chunk *)_source.allocate(size);

  if (!chunk) {
    return NULL;
  }

  chunk->_next = NULL;
  chunk->_idx = 0;
  chunk->_size = chunkSize;

  // Always keep the next available block aligned
  chunk->_data = ((char *)chunk) + SizeofChunk(_alignmentSize);

  return chunk;
}

CleanupHandler &DiscardAllocator::cleanupHandler() { return _cleanupHandler; }

bool DiscardAllocator::reallocates() { return false; }

void *DiscardAllocator::reallocate(void *block, UWord size) {
  assert(!"DiscardAllocators do not support reallocate");
  return NULL;
}

}  // namespace ESB
