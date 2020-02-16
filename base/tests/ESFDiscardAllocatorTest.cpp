/** @file ESFDiscardAllocatorTest.cpp
 *  @brief ESFDiscardAllocatorTest is the unit test for ESFDiscardAllocator.
 *
 *  Copyright 2005 Yahoo! Inc.
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:14 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_DISCARD_ALLOCATOR_TEST_H
#include <ESFDiscardAllocatorTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_DATE_H
#include <ESFDate.h>
#endif

static const int ChunkSize = 1024;
static const int Iterations = 10;
static const int AllocationsPerIteration = 10000;

static int GlobalUnsynchronizedCounter = 0;

ESFDiscardAllocatorTest::ESFDiscardAllocatorTest()
    : _rand(ESFDate::GetSystemTime().getMicroSeconds() +
            GlobalUnsynchronizedCounter++),
      _allocator(ChunkSize, ESFSystemAllocator::GetInstance()) {}

ESFDiscardAllocatorTest::~ESFDiscardAllocatorTest() {}

bool ESFDiscardAllocatorTest::run(ESTFResultCollector *collector) {
  ESFError error;
  char buffer[256];

  error = _allocator.initialize();

  if (ESF_SUCCESS != error) {
    ESFDescribeError(error, buffer, sizeof(buffer));

    ESTF_FAILURE(collector, buffer);
    ESTF_ERROR(collector, "Failed to initialize allocator");

    return false;
  }

  char *data = 0;
  int allocSize = 0;

  for (int i = 0; i < Iterations; ++i) {
    for (int j = 0; j < AllocationsPerIteration; ++j) {
      allocSize = generateAllocSize();
      data = (char *)_allocator.allocate(allocSize);

      ESTF_ASSERT(collector, data);

      for (int k = 0; k < allocSize; ++k) {
        data[k] = 'b';
      }
    }

    error = _allocator.reset();

    if (ESF_SUCCESS != error) {
      ESFDescribeError(error, buffer, sizeof(buffer));

      ESTF_FAILURE(collector, buffer);

      ESTF_ERROR(collector, "Failed to reset allocator");

      return false;
    }
  }

  error = _allocator.destroy();

  if (ESF_SUCCESS != error) {
    ESFDescribeError(error, buffer, sizeof(buffer));
    ESTF_FAILURE(collector, buffer);
    ESTF_ERROR(collector, "Failed to destroy allocator");

    return false;
  }

  ESTF_ASSERT(collector, ESF_SUCCESS == error);

  return true;
}

bool ESFDiscardAllocatorTest::setup() { return true; }

bool ESFDiscardAllocatorTest::tearDown() { return true; }

ESTFComponentPtr ESFDiscardAllocatorTest::clone() {
  ESTFComponentPtr component(new ESFDiscardAllocatorTest());

  return component;
}

int ESFDiscardAllocatorTest::generateAllocSize() {
  int uniformDeviate = _rand.generateRandom(1, 10);

  // 80% of allocations are less than 1/10 the chunksize

  if (9 > uniformDeviate) {
    return _rand.generateRandom(1, ChunkSize / 10);
  }

  // 10% of allocations are between 1/10 and the chunksize

  if (9 == uniformDeviate) {
    return _rand.generateRandom(ChunkSize / 10, ChunkSize);
  }

  // 10% exceed the chunksize

  return _rand.generateRandom(ChunkSize, ChunkSize * 2);
}
