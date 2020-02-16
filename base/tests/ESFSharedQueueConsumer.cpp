/**	@file ESFSharedQueueConsumer.cpp
 *	@brief ESFSharedQueueConsumer is part of the unit test for
 *ESFSharedQueue
 *
 *  Copyright 2005 Joshua Blatt
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

#ifndef ESF_SHARED_QUEUE_CONSUMER_H
#include <ESFSharedQueueConsumer.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

ESFSharedQueueConsumer::ESFSharedQueueConsumer(ESFSharedQueue &queue,
                                               ESFUInt32 items)
    : _items(items), _queue(queue) {}

ESFSharedQueueConsumer::~ESFSharedQueueConsumer() {}

bool ESFSharedQueueConsumer::run(ESTFResultCollector *collector) {
  int *item = 0;
  ESFUInt32 i = 0;
  ESFError error;
  ESTFRand rand;

  while (i < _items) {
    item = 0;

    if (1 == rand.generateRandom(1, 2)) {
      ESTF_ASSERT(collector, ESF_SUCCESS == _queue.pop((void **)&item));
      ESTF_ASSERT(collector, item);

      if (item) ++i;

      continue;
    }

    error = _queue.tryPop((void **)&item);

    ESTF_ASSERT(collector, ESF_SUCCESS == error || ESF_AGAIN == error);

    if (ESF_SUCCESS == error) {
      ESTF_ASSERT(collector, item);

      if (item) ++i;
    }
  }

  return true;
}

bool ESFSharedQueueConsumer::setup() { return true; }

bool ESFSharedQueueConsumer::tearDown() { return true; }

ESTFComponentPtr ESFSharedQueueConsumer::clone() {
  ESTFComponentPtr component(new ESFSharedQueueConsumer(_queue, _items));

  return component;
}
