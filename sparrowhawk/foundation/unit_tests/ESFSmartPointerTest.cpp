/**	@file ESFSmartPointerTest.cpp
 *	@brief ESFSmartPointerTest is the unit test for ESFSmartPointer and ESFReferenceCount.
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

#ifndef ESF_SMART_PTR_TEST_H
#include <ESFSmartPointerTest.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_OBJECT_PTR_DEBUGGER_H
#include <ESFSmartPointerDebugger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_FIXED_ALLOCATOR_H
#include <ESFFixedAllocator.h>
#endif

class ESFReferenceCountSubclass: public ESFReferenceCount {
public:

    ESFReferenceCountSubclass() :
        _theNumber(12) {
    }

    ~ESFReferenceCountSubclass() {
    }

    int getNumber() {
        return _theNumber;
    }

private:

    int _theNumber;
};

/** ESFReferenceCountSubclassPointer will be a subclass of ESFSmartPointer.
 *
 *  ESFReferenceCountSubclassPointer can be dereferenced to yield
 *  ESFReferenceCountSubclass instances...
 *
 *  ESFSmartPointer can be dereferenced to yield ESFReferenceCount instances.
 *
 *  ESFReferenceCountSubclass must be (and is) a subclass of ESFReferenceCount
 *  for this to work.  Also, ESFSmartPointer must (and does) encapsulate an
 *  ESFReferenceCount pointer.
 */
DEFINE_ESF_SMART_POINTER( ESFReferenceCountSubclass,
        ESFReferenceCountSubclassPointer,
        ESFSmartPointer );

ESFSmartPointerTest::ESFSmartPointerTest() {
}

ESFSmartPointerTest::~ESFSmartPointerTest() {
}

bool ESFSmartPointerTest::run(ESTFResultCollector *collector) {
    ESFSmartPointerDebugger *debugger = ESFSmartPointerDebugger::Instance();

    ESTFRand generator;

    ESFAllocator *source = ESFSystemAllocator::GetInstance();

    ESFReferenceCountSubclassPointer *array = (ESFReferenceCountSubclassPointer *) source->allocate(
            sizeof(ESFReferenceCountSubclassPointer) * 1000);

    ESFFixedAllocator allocator(1000, sizeof(ESFReferenceCountSubclass), source);

    int initialRefs = debugger->getSize();

    int activeRefs = initialRefs;

    int randIdx = 0;

    for (int i = 0; i < 10000; ++i) {
        randIdx = generator.generateRandom(0, 999);

        // Each iteration, 1/4 chance to delete, else, create.

        if (1 == generator.generateRandom(1, 4)) {
            if (array[randIdx].isNull()) {
                continue;
            }

            --activeRefs;

            array[randIdx].setNull();

            ESTF_ASSERT( collector, activeRefs == debugger->getSize() );

            continue;
        }

        if (array[randIdx].isNull()) {
            ++activeRefs;
        }

        // Alternate between the fixed length allocator and the system allocator
        // The smart pointer will keep track.

        if (1 == generator.generateRandom(1, 2)) {
            array[randIdx] = new (&allocator) ESFReferenceCountSubclass();
        } else {
            array[randIdx] = new ESFReferenceCountSubclass();
        }

        ESTF_ASSERT( collector, activeRefs == debugger->getSize() );

        ESTF_ASSERT( collector, 12 == array[randIdx]->getNumber() );
    }

    ESTF_ASSERT( collector, activeRefs == debugger->getSize() );

    for (int i = 0; i < 1000; ++i) {
        array[i].setNull();
    }

    ESTF_ASSERT( collector, initialRefs == debugger->getSize() );

    return true;
}

bool ESFSmartPointerTest::setup() {
    return true;
}

bool ESFSmartPointerTest::tearDown() {
    return true;
}

ESTFComponentPtr ESFSmartPointerTest::clone() {
    ESTFComponentPtr component(new ESFSmartPointerTest());

    return component;
}
