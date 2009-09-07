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

#ifndef ESF_LOCKABLE_TEST_H
#include <ESFLockableTest.h>
#endif

#ifndef ESF_SHARED_COUNTER_TEST_H
#include <ESFSharedCounterTest.h>
#endif

#ifndef ESF_BUDDY_ALLOCATOR_TEST_H
#include <ESFBuddyAllocatorTest.h>
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

#ifndef ESF_SMART_POINTER_TEST_H
#include <ESFSmartPointerTest.h>
#endif

int main() {
    ESFSmartPointerDebugger::Initialize();

    ESFSmartPointerTestPtr objectPtrTest = new ESFSmartPointerTest();

    ESTFCompositePtr testSuite = new ESTFComposite();

    testSuite->add(objectPtrTest);

    ESTFResultCollector collector;

    if (false == testSuite->setup()) {
        cerr << "Testing framework setup failed" << endl;
        return 1;
    }

    if (false == testSuite->run(&collector)) {
        cerr << "Testing framework run failed" << endl;
    }

    if (false == testSuite->tearDown()) {
        cerr << "Testing framework tear down failed" << endl;
    }

    if (0 == collector.getFailureCount() && 0 == collector.getErrorCount()) {
        cout << "All test cases passed" << endl;
    }

    cout << collector << endl;

    cout << "Remaining ESFObject references: " << ESFSmartPointerDebugger::Instance()->getSize() << endl;

    ESFSmartPointerDebugger::Destroy();

    return 0;
}

