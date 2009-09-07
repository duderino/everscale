/**	@file ESFSharedQueueTest.h
 *	@brief ESFSharedQueueTest is the unit test for ESFSharedQueue
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

#ifndef ESF_SHARED_QUEUE_TEST_H
#define ESF_SHARED_QUEUE_TEST_H

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

/**	ESTFSharedQueueTest is part of the unit test for ESFSharedQueue.
 *
 *  @ingroup foundation_test
 */
class ESFSharedQueueTest: public ESTFComponent {
public:

    /**	Constructor.
     */
    ESFSharedQueueTest();

    /** Destructor. */
    virtual ~ESFSharedQueueTest();

    /** Run the component.
     *
     *	@param collector A result collector that will collect the results of
     *		this test run.
     *	@return true if the test run was successfully performed by the test
     *		framework.  Application errors discovered during a test run do not
     *		count, a false return means there was an error in the test suite
     *		itself that prevented it from completing one or more test cases.
     */
    virtual bool run(ESTFResultCollector *collector);

    /** Perform a one-time initialization of the component.  Initializations
     *	that must be performed on every run of a test case should be put in
     *	the run method.
     *
     *	@return true if the one-time initialization was successfully performed,
     *		false otherwise.
     */
    virtual bool setup();

    /** Perform a one-time tear down of the component.  Tear downs that must be
     *	performed on every run of a test case should be put in the run method.
     *
     *	@return true if the one-time tear down was successfully performed,
     *		false otherwise.
     */
    virtual bool tearDown();

    /** Returns a deep copy of the component.
     *
     *	@return A deep copy of the component.
     */
    virtual ESTFComponentPtr clone();
};

DEFINE_ESTF_OBJECT_PTR(ESFSharedQueueTest,ESTFComponent)

#endif                                 /* ! ESF_SHARED_QUEUE_TEST_H */
