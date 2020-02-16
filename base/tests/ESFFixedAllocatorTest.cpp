/**	@file ESFFixedAllocatorTest.cpp
 *	@brief ESFFixedAllocatorTest is the unit test for ESFFixedAllocator.
 *
 *  Copyright 2005 Joshua Blatt, Yahoo! Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:14 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_FIXED_ALLOCATOR_TEST_H
#include <ESFFixedAllocatorTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

static const int Iterations = 2000;
static const int Allocations = 160;
static const bool Debug = false;
static const int Blocks = 160;
static const int BlockSize = 4096;

ESFFixedAllocatorTest::ESFFixedAllocatorTest()
    : _rand(),
      _allocator(Blocks, BlockSize, ESFSystemAllocator::GetInstance()) {}

ESFFixedAllocatorTest::~ESFFixedAllocatorTest() {}

bool ESFFixedAllocatorTest::run(ESTFResultCollector *collector) {
  ESFError error;
  char buffer[256];

  struct allocation {
    int _lifetime;
    void *_data;
  } allocations[Allocations];

  memset(allocations, 0, sizeof(allocations));

  for (int i = 0; i < Iterations; ++i) {
    for (int j = 0; j < Allocations; ++j) {
      if (allocations[j]._data && allocations[j]._lifetime == i) {
        if (Debug) {
          cerr << "Freeing block at time " << i << endl;
        }

        char *data = (char *)allocations[j]._data;

        // Make sure no one else overwrote this block.
        for (int k = 0; k < BlockSize; ++k) {
          ESTF_ASSERT(collector, (i % 127) == data[k]);
        }

        error = _allocator.deallocate(allocations[j]._data);
        allocations[j]._data = 0;

        if (ESF_SUCCESS != error) {
          ESFDescribeError(error, buffer, sizeof(buffer));
          ESTF_FAILURE(collector, buffer);
        }

        ESTF_ASSERT(collector, ESF_SUCCESS == error);
        ESTF_ASSERT(collector, !allocations[j]._data);

        allocations[j]._lifetime = 0;
        allocations[j]._data = 0;
      }

      if (!allocations[j]._data && 1 == _rand.generateRandom(1, 4)) {
        allocations[j]._lifetime = i + generateAllocLifetime();

        if (Debug) {
          cerr << "Allocating block at time " << i << " with lifetime "
               << allocations[j]._lifetime;
        }

        allocations[j]._data = _allocator.allocate(BlockSize);

        if (0 == allocations[j]._data) {
          ESFDescribeError(ESF_OUT_OF_MEMORY, buffer, sizeof(buffer));
          ESTF_FAILURE(collector, buffer);
        }

        ESTF_ASSERT(collector, allocations[j]._data);

        if (!allocations[j]._data) {
          if (Debug) {
            cerr << "Failed to allocate block at time " << i;
          }

          allocations[j]._lifetime = 0;
        } else {
          //
          // We will check this value when we free the block to
          // make sure no one overwrote it.
          //
          memset(allocations[j]._data, allocations[j]._lifetime % 127,
                 BlockSize);
        }
      }
    }
  }

  for (int i = 0; i < Allocations; ++i) {
    if (allocations[i]._data) {
      error = _allocator.destroy();

      ESTF_ASSERT(collector, ESF_IN_USE == error);

      break;
    }
  }

  // Simulation mostly done, return everything to the allocator.

  for (int i = 0; i < Allocations; ++i) {
    if (allocations[i]._data) {
      if (Debug) {
        cerr << "Freeing block at cleanup stage\n";
      }

      error = _allocator.deallocate(allocations[i]._data);
      allocations[i]._data = 0;

      if (ESF_SUCCESS != error) {
        ESFDescribeError(error, buffer, sizeof(buffer));
        ESTF_FAILURE(collector, buffer);
      }

      ESTF_ASSERT(collector, ESF_SUCCESS == error);
      ESTF_ASSERT(collector, !allocations[i]._data);

      allocations[i]._lifetime = 0;
      allocations[i]._data = 0;
    }
  }

  error = _allocator.destroy();

  if (ESF_SUCCESS != error) {
    ESFDescribeError(error, buffer, sizeof(buffer));
    ESTF_FAILURE(collector, buffer);
    ESTF_ERROR(collector, "Failed to destroy allocator");
    return false;
  }

  return true;
}

bool ESFFixedAllocatorTest::setup() { return true; }

bool ESFFixedAllocatorTest::tearDown() { return true; }

ESTFComponentPtr ESFFixedAllocatorTest::clone() {
  ESTFComponentPtr component(new ESFFixedAllocatorTest());

  return component;
}

int ESFFixedAllocatorTest::generateAllocLifetime() {
  int uniformDeviate = _rand.generateRandom(1, 3);

  switch (uniformDeviate) {
    case 1:
      return _rand.generateRandom(1, 10);

    case 2:
      return _rand.generateRandom(1, 100);

    case 3:
      return _rand.generateRandom(1, 1000);
  }

  return 10;
}
