/**	@file ESFSharedCounterTest.cpp
 *	@brief ESFSharedCounterTest is the unit test for the ESFSharedCounter
 *class.
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

#ifndef ESF_SHARED_COUNTER_TEST_H
#include <ESFSharedCounterTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

ESFSharedCounter ESFSharedCounterTest::_Counter;
int ESFSharedCounterTest::_UnprotectedCounter;

ESFSharedCounterTest::ESFSharedCounterTest() {}

ESFSharedCounterTest::~ESFSharedCounterTest() {}

bool ESFSharedCounterTest::run(ESTFResultCollector *collector) {
  int value = 0;

  for (int i = 0; i < 100000; ++i) {
    _Counter.add(1);

    ++_UnprotectedCounter;

    value = _Counter.inc();

    ++_UnprotectedCounter;

    // fprintf(stderr, "Value: %d, shared counter: %d, unprotected counter
    // %d\n", value, _Counter.get(), _UnprotectedCounter);
  }

  fprintf(stderr, "Value: %d, Shared counter: %d, unprotected counter %d\n",
          value, _Counter.get(), _UnprotectedCounter);

  for (int i = 0; i < 100000; ++i) {
    _Counter.sub(1);

    --_UnprotectedCounter;

    value = _Counter.dec();

    --_UnprotectedCounter;
  }

  fprintf(stderr, "Value: %d, Shared counter: %d, unprotected counter %d\n",
          value, _Counter.get(), _UnprotectedCounter);

  return true;
}

bool ESFSharedCounterTest::setup() { return true; }

bool ESFSharedCounterTest::tearDown() { return true; }

ESTFComponentPtr ESFSharedCounterTest::clone() {
  ESTFComponentPtr component(this);

  return component;
}
