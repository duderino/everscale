/**	@file ESFSharedQueueTest.cpp
 *	@brief ESFSharedQueueTest is part of the unit test for ESFSharedQueue
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

#ifndef ESF_SHARED_QUEUE_TEST_H
#include <ESFSharedQueueTest.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_SHARED_QUEUE_PRODUCER_H
#include <ESFSharedQueueProducer.h>
#endif

#ifndef ESF_SHARED_QUEUE_CONSUMER_H
#include <ESFSharedQueueConsumer.h>
#endif

#ifndef ESF_SHARED_QUEUE_H
#include <ESFSharedQueue.h>
#endif

#ifndef ESTF_CONCURRENCY_COMPOSITE_H
#include <ESTFConcurrencyComposite.h>
#endif

ESFSharedQueueTest::ESFSharedQueueTest() {}

ESFSharedQueueTest::~ESFSharedQueueTest() {}

bool ESFSharedQueueTest::run(ESTFResultCollector *collector) {
  ESTFComponentPtr component;
  ESFSharedQueue queue(ESFSystemAllocator::GetInstance(), 20);
  ESTFConcurrencyComposite composite;

  for (int i = 0; i < 9; ++i) {
    component = new ESFSharedQueueProducer(i, queue, ESF_UINT32_C(10000));

    composite.add(component);
  }

  for (int i = 0; i < 3; ++i) {
    component = new ESFSharedQueueConsumer(queue, ESF_UINT32_C(30000));

    composite.add(component);
  }

  bool result = composite.run(collector);

  composite.clear();

  return result;
}

bool ESFSharedQueueTest::setup() { return true; }

bool ESFSharedQueueTest::tearDown() { return true; }

ESTFComponentPtr ESFSharedQueueTest::clone() {
  ESTFComponentPtr component(new ESFSharedQueueTest());

  return component;
}
