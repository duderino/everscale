/**    @file ESTFConcurrencyDecorator.h
 *  @brief ESTFConcurrencyDecorators run their decorated test case in multiple
 *      threads.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:19 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESTF_CONCURRENCY_DECORATOR_H
#define ESTF_CONCURRENCY_DECORATOR_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

/** ESTFConcurrencyDecorators run their decorated test case in multiple threads.
 *  When the run method of a ESTFConcurrencyDecorator is called, it will
 *  spawn several new threads to run its decorated component concurrently.
 *  The ESTFConcurrencyDecorator will also merge the results of the different
 *  threads test runs into a single result collector.
 *
 *  @ingroup test
 */
class ESTFConcurrencyDecorator : public ESTFComponent
{
public:

    /** Constructor.
      *
     *    @param component The decorated component.
     *    @param threads The number of threads with which the decorated
     *        component will be run concurrently.
     */
    ESTFConcurrencyDecorator( ESTFComponentPtr &component, int threads );

    /** Destructor.
     */
    virtual ~ESTFConcurrencyDecorator();

    /** Run the decorator.  Will spawn a number of threads as set in the
     *    constructor to run the decorated component's run method concurrently.
     *
     *    @param collector A result collector that will collect the results of
     *        this test run.
     *    @return true if the test run was successfully performed by the test
     *        framework.  Application errors discovered during a test run do not
     *        count, a false return means there was an error in the test suite
     *        itself that prevented it from completing one or more test cases.
     */
    virtual bool run( ESTFResultCollector *collector );

    /** Perform a one-time initialization of the test case.  Initializations
     *    that must be performed on every run of a test case should be put in
     *    the run method.  This will simply forward the setup call to the
     *    decorated component.
     *
     *    @return true if the one-time initialization was successfully performed,
     *        false otherwise.
     */
    virtual bool setup();

    /** Perform a one-time tear down of the test case.  Tear downs that must be
     *    performed on every run of a test case should be put in the run method.
     *    This will simply forward the tearDown call to the decorated component.
     *
     *    @return true if the one-time tear down was successfully performed,
     *        false otherwise.
     */
    virtual bool tearDown();

    /** Returns a deep copy of the component.  This will return a deep copy of
     *    the decorator and a deep copy of the decorated component.
     *
     *    @return A deep copy of the component.
     */
    virtual ESTFComponentPtr clone();

private:

    ESTFComponentPtr _component;
    int _threads;
};

DEFINE_ESTF_OBJECT_PTR(ESTFConcurrencyDecorator,ESTFComponent)

#endif                                 /* ! ESTF_CONCURRENCY_DECORATOR_H */
