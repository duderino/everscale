/** @file ESTFRandomizedDecorator.h
 *  @brief ESTFRandomizedDecorators run their decorated test case at a
 *      configurable probability.
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

#ifndef ESTF_RANDOMIZED_DECORATOR_H
#define ESTF_RANDOMIZED_DECORATOR_H

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

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

/** ESTFRandomizedDecorators call their decorated component's run method
 *  with a probability of 1/n where n is a user supplied parameter.
 *
 *  One use would be to decorate m test cases with their own randomized
 *  decorators.  All the decorators could be added to a composite which could
 *  then be decorated in turn by a repetition decorator.  On every repetition,
 *  each test case in the composite might or might not be called.  The end
 *  result would be a more or less random intermixing of the test cases in the
 *  composite.
 *
 *  @ingroup test
 */
class ESTFRandomizedDecorator : public ESTFComponent
{
public:

    /** Constructor.
      *
     *    @param component The decorated component.
     *    @param prob The 1/prob chance the decorated test case will be
     *        run when the decorator's run method is called.
     */
    ESTFRandomizedDecorator( ESTFComponentPtr &component, int prob );

    /** Destructor.
     */
    virtual ~ESTFRandomizedDecorator();

    /** Run the decorator.  Will randomly decide whether the decorated
     *    instance's run method should in turn be called.
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
    ESTFRand _rand;
    int _prob;

};

DEFINE_ESTF_OBJECT_PTR(ESTFRandomizedDecorator,ESTFComponent)

#endif                                 /* ! ESTF_RANDOMIZED_DECORATOR_H */
