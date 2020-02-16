/** @file ESFBufferTestMain.cpp
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
    ESFBufferTestPtr bufferTest = new ESFBufferTest();

    ESTFConcurrencyDecoratorPtr concurrencyDecorator = new ESTFConcurrencyDecorator(bufferTest, 3);

    ESTFCompositePtr composite = new ESTFComposite();

    composite->add(concurrencyDecorator);

    ESTFRepetitionDecoratorPtr repetitionDecorator = new ESTFRepetitionDecorator(composite, 3);

    ESTFResultCollector collector;

    if (false == repetitionDecorator->setup()) {
        cerr << "Testing framework setup failed" << endl;
        return 1;
    }

    if (false == repetitionDecorator->run(&collector)) {
        cerr << "Testing framework run failed" << endl;
    }

    if (false == repetitionDecorator->tearDown()) {
        cerr << "Testing framework tear down failed" << endl;
    }

    if (0 == collector.getFailureCount() && 0 == collector.getErrorCount()) {
        cout << "All test cases passed" << endl;
    }

    cout << collector << endl;

    return 0;
}
