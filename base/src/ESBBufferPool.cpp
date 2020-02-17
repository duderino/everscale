#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

namespace ESB {

#define MIN_NUMBER_OF_BUFFERS 1
#define MIN_BUFFER_SIZE 1

BufferPool::BufferPool(int numberOfBuffers, int bufferSize,
                       Allocator *allocator)
    : _numberOfBuffers(numberOfBuffers < MIN_NUMBER_OF_BUFFERS
                           ? MIN_NUMBER_OF_BUFFERS
                           : numberOfBuffers),
      _bufferSize(bufferSize < MIN_BUFFER_SIZE ? MIN_BUFFER_SIZE : bufferSize),
      _listSize(0),
      _allocator(allocator ? allocator : SystemAllocator::GetInstance()),
      _embeddedList(),
      _lock() {}

BufferPool::~BufferPool() {
  for (EmbeddedListElement *element = _embeddedList.removeFirst(); element;
       element = _embeddedList.removeFirst()) {
    element->~EmbeddedListElement();
    _allocator->deallocate(element);
    --_listSize;
  }

  assert(0 == _listSize);
}

Buffer *BufferPool::acquireBuffer() {
  WriteScopeLock scopeLock(_lock);

  Buffer *buffer = (Buffer *)_embeddedList.removeLast();

  if (buffer) {
    --_listSize;

    return buffer;
  }

  return Buffer::Create(_allocator, _bufferSize);
}

void BufferPool::releaseBuffer(Buffer *buffer) {
  WriteScopeLock scopeLock(_lock);

  if (_listSize >= _numberOfBuffers) {
    buffer->~Buffer();
    _allocator->deallocate(buffer);

    return;
  }

  buffer->clear();
  _embeddedList.addLast(buffer);
  ++_listSize;
}

}  // namespace ESB
