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

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_READ_WRITE_LOCK_H
#include <ESFReadWriteLock.h>
#endif

#ifndef ESF_COUNTING_SEMAPHORE_H
#include <ESFCountingSemaphore.h>
#endif

#ifndef ESF_NULL_LOCK_H
#include <ESFNullLock.h>
#endif

#ifndef ESF_LOCKABLE_TEST_H
#include <ESFLockableTest.h>
#endif

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

int main() {
  ESFMutex mutex;
  ESFReadWriteLock rwLock;
  ESFCountingSemaphore countingSemaphore;

  countingSemaphore.writeRelease();  // Using like a binary semaphore

  ESFLockableTestPtr mutexTest = new ESFLockableTest(mutex);
  ESFLockableTestPtr rwLockTest = new ESFLockableTest(rwLock);
  ESFLockableTestPtr semaphoreTest = new ESFLockableTest(countingSemaphore);

  ESTFConcurrencyDecoratorPtr mutexDecorator =
      new ESTFConcurrencyDecorator(mutexTest, 10);
  ESTFConcurrencyDecoratorPtr rwLockDecorator =
      new ESTFConcurrencyDecorator(rwLockTest, 10);
  ESTFConcurrencyDecoratorPtr semaphoreDecorator =
      new ESTFConcurrencyDecorator(semaphoreTest, 10);

  ESTFCompositePtr testSuite = new ESTFComposite();

  testSuite->add(mutexDecorator);
  testSuite->add(rwLockDecorator);
  testSuite->add(semaphoreDecorator);

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
