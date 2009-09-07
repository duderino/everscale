/**	@file ESFLockableTest.cpp
 *  @brief ESFLockableTest is the unit test for the ESFLockable class.
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

#ifndef ESF_LOCKABLE_TEST_H
#include <ESFLockableTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESTF_THREAD_H
#include <ESTFThread.h>
#endif

ESFLockableTest::ESFLockableTest(ESFLockable &lockable) :
    ESTFComponent(), _lock(lockable), _counter1(0), _counter2(0) {
}

ESFLockableTest::~ESFLockableTest() {
}

bool ESFLockableTest::run(ESTFResultCollector *collector) {
    int counter1;
    ESFError error;
    char buffer[256];

    for (int i = 0; i < 100; ++i) {
        error = _lock.readAcquire();

        if (ESF_SUCCESS != error) {
            ESFDescribeError(error, buffer, sizeof(buffer));
            ESTF_FAILURE( collector, buffer );
        }

        ESTF_ASSERT( collector, ESF_SUCCESS == error );

        for ( int i = 0; i < 10; ++i )
        {
            counter1 = _counter1;

            ESTFThread::Yield();

            ESTF_ASSERT( collector, counter1 == _counter2 );
        }

        error = _lock.readRelease();

        if ( ESF_SUCCESS != error )
        {
            ESFDescribeError( error, buffer, sizeof( buffer ) );
            ESTF_FAILURE( collector, buffer );
        }

        error = _lock.writeAcquire();

        if ( ESF_SUCCESS != error )
        {
            ESFDescribeError( error, buffer, sizeof( buffer ) );
            ESTF_FAILURE( collector, buffer );
        }

        ++_counter1;

        ESTFThread::Yield();

        ++_counter2;

        ESTF_ASSERT( collector, _counter1 == _counter2 );

        error = _lock.writeRelease();

        if ( ESF_SUCCESS != error )
        {
            ESFDescribeError( error, buffer, sizeof( buffer ) );
            ESTF_FAILURE( collector, buffer );
        }
    }

    return true;
}

bool ESFLockableTest::setup() {
    _counter1 = 0;
    _counter2 = 0;

    return true;
}

bool ESFLockableTest::tearDown() {
    return true;
}

ESTFComponentPtr ESFLockableTest::clone() {
    //
    //	Do a shallow clone so cloned instances will share the lock and the
    //	counters when they are wrapped in the concurrency decorator.
    //
    ESTFComponentPtr component(this);

    return component;
}
