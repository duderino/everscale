/** @file ESFBufferPool.cpp
 *  @brief A synchronized pool of ESFBuffers
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_BUFFER_POOL_H
#include <ESFBufferPool.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#define MIN_NUMBER_OF_BUFFERS 1
#define MIN_BUFFER_SIZE 1

ESFBufferPool::ESFBufferPool(int numberOfBuffers, int bufferSize, ESFAllocator *allocator) :
    _numberOfBuffers(numberOfBuffers < MIN_NUMBER_OF_BUFFERS ? MIN_NUMBER_OF_BUFFERS : numberOfBuffers), _bufferSize(
            bufferSize < MIN_BUFFER_SIZE ? MIN_BUFFER_SIZE : bufferSize), _listSize(0), _allocator(
            allocator ? allocator : ESFSystemAllocator::GetInstance()), _embeddedList(), _lock() {
}

ESFBufferPool::~ESFBufferPool() {
    for (ESFEmbeddedListElement *element = _embeddedList.removeFirst(); element; element = _embeddedList.removeFirst()) {
        element->~ESFEmbeddedListElement();
        _allocator->deallocate(element);
        --_listSize;
    }

    ESF_ASSERT(0 == _listSize);
}

ESFBuffer *ESFBufferPool::acquireBuffer() {
    ESFWriteScopeLock scopeLock(_lock);

    ESFBuffer *buffer = (ESFBuffer *) _embeddedList.removeLast();

    if (buffer) {
        --_listSize;

        return buffer;
    }

    return ESFBuffer::Create(_allocator, _bufferSize);
}

void ESFBufferPool::releaseBuffer(ESFBuffer *buffer) {
    ESFWriteScopeLock scopeLock(_lock);

    if (_listSize >= _numberOfBuffers) {
        buffer->~ESFBuffer();
        _allocator->deallocate(buffer);

        return;
    }

    buffer->clear();
    _embeddedList.addLast(buffer);
    ++_listSize;
}

