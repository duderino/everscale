/** @file ESTFRepetitionDecorator.h
 *  @brief ESTFRepetitionDecorators run their decorated test case for a
 *      configurable number of repetitions.
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

#ifndef ESTF_REPETITION_DECORATOR_H
#define ESTF_REPETITION_DECORATOR_H

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

/**    Whenever the run method of a ESTFRepetitionDecorator is called, it will
 *  call the run method of its decorated component for a configurable number
 *  of repetitions.
 *
 *  @ingroup test
 */
class ESTFRepetitionDecorator : public ESTFComponent
{
public:

    /** Constructor.
      *
     *    @param component The decorated component.
     *    @param repetitions The number of times the decorated component's run
     *        method will be called every time the decorator's run method is
     *        called.
     */
    ESTFRepetitionDecorator( ESTFComponentPtr &component, int repetitions );

    /** Destructor.
     */
    virtual ~ESTFRepetitionDecorator();

    /** Run the decorator.  Will call the decorated component's run method for
     *    the number of repetitions set in the constructor.
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
    int _repetitions;

};

DEFINE_ESTF_OBJECT_PTR(ESTFRepetitionDecorator,ESTFComponent)

#endif                                 /* ! ESTF_REPETITION_DECORATOR_H */
