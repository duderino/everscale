#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

namespace ESB {

BufferPool::BufferPool(UInt32 bufferSize, UInt32 maxBuffers, Lockable &lock,
                       Allocator &allocator)
    : _maxBuffers(maxBuffers),
      _bufferSize(bufferSize),
      _listSize(0U),
      _lock(lock),
      _allocator(allocator),
      _embeddedList() {}

BufferPool::~BufferPool() {
  for (EmbeddedListElement *element = _embeddedList.removeFirst(); element;
       element = _embeddedList.removeFirst()) {
    element->~EmbeddedListElement();
    _allocator.deallocate(element);
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

  if (0 < _maxBuffers && _listSize >= _maxBuffers) {
    buffer->~Buffer();
    _allocator.deallocate(buffer);
    return;
  }

  buffer->clear();
  _embeddedList.addLast(buffer);
  ++_listSize;
}

}  // namespace ESB
