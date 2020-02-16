/** @file ESFBufferTest.cpp
 *  @brief ESFBufferTest is the unit test for ESFBuffer.
 *
 *  Copyright 2005 Yahoo! Inc.
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:14 $
 *  $Name:  $
 *  $Revision: 1.2 $
 */

#ifndef ESF_BUFFER_TEST_H
#include <ESFBufferTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

static const int BufferSize = 4096;
static const int Iterations = 10000;
static const bool Debug = false;

ESFBufferTest::ESFBufferTest() :
    _capacity(BufferSize), _rand(), _buffer((unsigned char *) ESFSystemAllocator::GetInstance()->allocate(BufferSize),
            BufferSize) {
}

ESFBufferTest::~ESFBufferTest() {
}

void ESFBufferTest::fillTest(ESTFResultCollector *collector, unsigned int startPosition, unsigned int endPosition) {
    _buffer.setWritePosition(startPosition);

    ESTF_ASSERT( collector, _capacity == _buffer.getCapacity() );
    ESTF_ASSERT( collector, _capacity - startPosition == _buffer.getWritable() );

    if (0 < _capacity - startPosition) {
        ESTF_ASSERT( collector, true == _buffer.isWritable() );
    } else {
        ESTF_ASSERT( collector, false == _buffer.isWritable() );
    }

    for (unsigned int i = startPosition; i < endPosition; ++i) {
        ESTF_ASSERT( collector, true == _buffer.isWritable() );

        _buffer.putNext('a');

        ESTF_ASSERT( collector, _capacity == _buffer.getCapacity() );
        ESTF_ASSERT( collector, _capacity - i - 1 == _buffer.getWritable() );
        ESTF_ASSERT( collector, i + 1 == _buffer.getWritePosition() );
    }

    ESTF_ASSERT( collector, _capacity == _buffer.getCapacity() );
    ESTF_ASSERT( collector, _capacity - endPosition == _buffer.getWritable() );
    ESTF_ASSERT( collector, endPosition == _buffer.getWritePosition() );

    if (0 < _capacity - endPosition) {
        ESTF_ASSERT( collector, true == _buffer.isWritable() );
    } else {
        ESTF_ASSERT( collector, false == _buffer.isWritable() );
    }
}

void ESFBufferTest::drainTest(ESTFResultCollector *collector, unsigned int startPosition, unsigned int endPosition) {
    _buffer.setReadPosition(startPosition);

    ESTF_ASSERT( collector, _capacity == _buffer.getCapacity() );
    ESTF_ASSERT( collector, endPosition - startPosition == _buffer.getReadable() );

    if (0 < endPosition - startPosition) {
        ESTF_ASSERT( collector, true == _buffer.isReadable() );
    } else {
        ESTF_ASSERT( collector, false == _buffer.isReadable() );
    }

    for (unsigned int i = startPosition; i < endPosition; ++i) {
        ESTF_ASSERT( collector, true == _buffer.isReadable() );

        ESTF_ASSERT( collector, 'a' == _buffer.getNext() );;

        ESTF_ASSERT( collector, _capacity == _buffer.getCapacity() );
        ESTF_ASSERT( collector, endPosition - i - 1 == _buffer.getReadable() );
        ESTF_ASSERT( collector, i + 1 == _buffer.getReadPosition() );
    }

    ESTF_ASSERT( collector, _capacity == _buffer.getCapacity() );
    ESTF_ASSERT( collector, 0 == _buffer.getReadable() );
    ESTF_ASSERT( collector, false == _buffer.isReadable() );
}

bool ESFBufferTest::run(ESTFResultCollector *collector) {
    fillTest(collector, 0, _capacity);

    drainTest(collector, 0, _capacity);

    _buffer.clear();

    int startPosition = 0;
    int endPosition = _capacity;

    for (int i = 0; i < Iterations; ++i) {
        endPosition = _rand.generateRandom(2, _capacity);
        startPosition = _rand.generateRandom(0, endPosition - 1);

        fillTest(collector, startPosition, endPosition);

        drainTest(collector, startPosition, endPosition);

        _buffer.compact();
    }

    return true;
}

bool ESFBufferTest::setup() {
    return true;
}

bool ESFBufferTest::tearDown() {
    return true;
}

ESTFComponentPtr ESFBufferTest::clone() {
    ESTFComponentPtr component(new ESFBufferTest());

    return component;
}

