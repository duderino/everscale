/** @file ESFDiscardAllocator.cpp
 *  @brief An allocator good for lots of small allocations with the same
 *      lifetime.
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

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#define ESF_MIN_BLOCK_SIZE 128

ESFDiscardAllocator::ESFDiscardAllocator(int chunkSize, ESFAllocator *source) :
    _head(0), _chunkSize(ESF_MIN_BLOCK_SIZE > chunkSize ? ESF_MIN_BLOCK_SIZE : chunkSize), _source(source ? source : ESFSystemAllocator::GetInstance()) {
}

ESFDiscardAllocator::ESFDiscardAllocator() :
    _head(0), _chunkSize(ESF_MIN_BLOCK_SIZE), _source(ESFSystemAllocator::GetInstance()) {
}

ESFError ESFDiscardAllocator::initialize(int chunkSize, ESFAllocator *source) {
    _chunkSize = ESF_MIN_BLOCK_SIZE > chunkSize ? ESF_MIN_BLOCK_SIZE : chunkSize;
    _source = source ? source : ESFSystemAllocator::GetInstance();

    return ESF_SUCCESS;
}

ESFDiscardAllocator::~ESFDiscardAllocator() {
    destroy();
}

void *ESFDiscardAllocator::allocate(ESFUWord size) {
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
    _head->_idx += ESF_WORD_ALIGN(size);

    return block;
}

ESFError ESFDiscardAllocator::deallocate(void *block) {
    return block ? ESF_SUCCESS : ESF_NULL_POINTER;
}

ESFUWord ESFDiscardAllocator::getOverhead() {
    return 0;
}

ESFError ESFDiscardAllocator::initialize() {
    return ESF_SUCCESS;
}

ESFError ESFDiscardAllocator::destroy() {
    Chunk *current = _head;
    Chunk *next = 0;

    while (current) {
        next = current->_next;

        _source->deallocate(current);

        current = next;
    }

    _head = 0;

    return ESF_SUCCESS;
}

ESFError ESFDiscardAllocator::reset() {
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

    return ESF_SUCCESS;
}

ESFError ESFDiscardAllocator::isInitialized() {
    return ESF_SUCCESS;
}

ESFError ESFDiscardAllocator::setFailoverAllocator(ESFAllocator *allocator) {
    return allocator ? ESF_SUCCESS : ESF_NULL_POINTER;
}

ESFError ESFDiscardAllocator::getFailoverAllocator(ESFAllocator **allocator) {
    if (!allocator) {
        return ESF_NULL_POINTER;
    }

    *allocator = _source;

    return ESF_SUCCESS;
}

ESFSize ESFDiscardAllocator::GetOverheadSize() {
    return ESF_WORD_ALIGN(sizeof(Chunk));
}

ESFDiscardAllocator::Chunk *ESFDiscardAllocator::allocateChunk(int chunkSize) {
    Chunk *tmp = (Chunk *) _source->allocate(ESF_WORD_ALIGN(sizeof(Chunk)) + chunkSize);

    if (0 == tmp) {
        return 0;
    }

    tmp->_next = 0;
    tmp->_idx = 0;
    tmp->_size = chunkSize;

    // Always keep the next available block word-aligned
    tmp->_data = ((char *) tmp) + ESF_WORD_ALIGN(sizeof(Chunk));

    return tmp;
}
