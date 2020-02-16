/*
 *  Copyright 2005 Yahoo! Inc.
 */

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_TEST_H
#include <ESFDiscardAllocatorTest.h>
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
    ESFDiscardAllocatorTestPtr discardAllocatorTest = new ESFDiscardAllocatorTest();

    ESTFConcurrencyDecoratorPtr discardAllocatorDecorator = new ESTFConcurrencyDecorator(discardAllocatorTest, 3);

    ESTFCompositePtr testSuite = new ESTFComposite();

    testSuite->add(discardAllocatorDecorator);

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

