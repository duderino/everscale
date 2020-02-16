/*
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
 */

#ifndef ESTF_CONCURRENCY_DECORATOR_H
#include <ESTFConcurrencyDecorator.h>
#endif

#ifndef ESTF_REPETITION_DECORATOR_H
#include <ESTFRepetitionDecorator.h>
#endif

#ifndef ESTF_RESULT_COLLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_COMPOSITE_H
#include <ESTFComposite.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_SHARED_QUEUE_TEST
#include <ESFSharedQueueTest.h>
#endif

int main() {
  ESFSharedQueueTestPtr sharedQueueTest = new ESFSharedQueueTest();

  ESTFConcurrencyDecoratorPtr sharedQueueDecorator =
      new ESTFConcurrencyDecorator(sharedQueueTest, 3);

  ESTFCompositePtr testSuite = new ESTFComposite();

  testSuite->add(sharedQueueDecorator);

  ESTFRepetitionDecoratorPtr root = new ESTFRepetitionDecorator(testSuite, 3);

  ESTFResultCollector collector;

  if (false == root->setup()) {
    cerr << "Testing framework setup failed" << endl;
    return 1;
  }

  if (false == root->run(&collector)) {
    cerr << "Testing framework run failed" << endl;
  }

  if (false == root->tearDown()) {
    cerr << "Testing framework tear down failed" << endl;
  }

  if (0 == collector.getFailureCount() && 0 == collector.getErrorCount()) {
    cout << "All test cases passed" << endl;
  }

  cout << collector << endl;

  return 0;
}
