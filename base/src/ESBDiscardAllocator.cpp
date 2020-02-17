#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

#define ESB_MIN_BLOCK_SIZE 128

DiscardAllocator::DiscardAllocator(int chunkSize, Allocator *source)
    : _head(0),
      _chunkSize(ESB_MIN_BLOCK_SIZE > chunkSize ? ESB_MIN_BLOCK_SIZE
                                                : chunkSize),
      _source(source ? source : SystemAllocator::GetInstance()) {}

DiscardAllocator::DiscardAllocator()
    : _head(0),
      _chunkSize(ESB_MIN_BLOCK_SIZE),
      _source(SystemAllocator::GetInstance()) {}

Error DiscardAllocator::initialize(int chunkSize, Allocator *source) {
  _chunkSize = ESB_MIN_BLOCK_SIZE > chunkSize ? ESB_MIN_BLOCK_SIZE : chunkSize;
  _source = source ? source : SystemAllocator::GetInstance();

  return ESB_SUCCESS;
}

DiscardAllocator::~DiscardAllocator() { destroy(); }

void *DiscardAllocator::allocate(UWord size) {
  if (1 > size) {
    return 0;
  }

  //
  // If asked for a block of memory larger than the chunk size, bypass
  // the chunks and allocate memory directly from the source allocator,
  // but remember the allocated memory so it can be freed when this
  // allocator is destroyed.
  //

  if (size > _chunkSize) {
    Chunk *chunk = allocateChunk(size);

    if (0 == chunk) {
      return 0;
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

    if (0 == _head) {
      return 0;
    }
  }

  if (size > (_head->_idx > _head->_size ? 0 : _head->_size - _head->_idx)) {
    Chunk *oldHead = _head;

    _head = allocateChunk(_chunkSize);

    if (0 == _head) {
      _head = oldHead;
      return 0;
    }

    _head->_next = oldHead;
  }

  void *block = _head->_data + _head->_idx;

  // Always keep the next available block word-aligned
  _head->_idx += ESB_WORD_ALIGN(size);

  return block;
}

Error DiscardAllocator::deallocate(void *block) {
  return block ? ESB_SUCCESS : ESB_NULL_POINTER;
}

UWord DiscardAllocator::getOverhead() { return 0; }

Error DiscardAllocator::initialize() { return ESB_SUCCESS; }

Error DiscardAllocator::destroy() {
  Chunk *current = _head;
  Chunk *next = 0;

  while (current) {
    next = current->_next;

    _source->deallocate(current);

    current = next;
  }

  _head = 0;

  return ESB_SUCCESS;
}

Error DiscardAllocator::reset() {
  Chunk *current = _head;
  Chunk *next = 0;

  while (current && current->_next) {
    next = current->_next;

    _source->deallocate(current);

    current = next;
  }

  _head = current;

  if (_head) {
    _head->_idx = 0;
  }

  return ESB_SUCCESS;
}

Error DiscardAllocator::isInitialized() { return ESB_SUCCESS; }

Error DiscardAllocator::setFailoverAllocator(Allocator *allocator) {
  return allocator ? ESB_SUCCESS : ESB_NULL_POINTER;
}

Error DiscardAllocator::getFailoverAllocator(Allocator **allocator) {
  if (!allocator) {
    return ESB_NULL_POINTER;
  }

  *allocator = _source;

  return ESB_SUCCESS;
}

Size DiscardAllocator::GetOverheadSize() {
  return ESB_WORD_ALIGN(sizeof(Chunk));
}

DiscardAllocator::Chunk *DiscardAllocator::allocateChunk(int chunkSize) {
  Chunk *tmp =
      (Chunk *)_source->allocate(ESB_WORD_ALIGN(sizeof(Chunk)) + chunkSize);

  if (0 == tmp) {
    return 0;
  }

  tmp->_next = 0;
  tmp->_idx = 0;
  tmp->_size = chunkSize;

  // Always keep the next available block word-aligned
  tmp->_data = ((char *)tmp) + ESB_WORD_ALIGN(sizeof(Chunk));

  return tmp;
}

}  // namespace ESB
