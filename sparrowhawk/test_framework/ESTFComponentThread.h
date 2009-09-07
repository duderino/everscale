/** @file ESTFComponentThread.h
 *  @brief A thread that runs a ESTFComponent
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

#ifndef ESTF_COMPONENT_THREAD_H
#define ESTF_COMPONENT_THREAD_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifndef ESTF_THREAD_H
#include <ESTFThread.h>
#endif

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

/** ESTFComponentThreads run a ESTFComponent in a new thread of control.
 *
 *  @ingroup test
 */
class ESTFComponentThread : public ESTFThread
{
public:
    /** Create a new instance */
    ESTFComponentThread();

    /** Default destructor */
    virtual ~ESTFComponentThread();

    /** Assign a ESTFComponent to be run in this thread.
     *
     *  @param component The component to be run in this thread
     */
    void setComponent( ESTFComponentPtr &component );

    /** Assign a collector to collect the results of the test run for this
     *  thread.
     *
     *  @param collector The collector that will collect the results of the
     *      component's test run.
     */
    void setCollector( ESTFResultCollector *collector );

    /** Get the result of the test run (i.e., did the framework itself fail)
     *
     *  @return the result of the test run (whether the framework itself
     *      succeeded).
     */
    bool getResult();

    /** Get the result collector assigned to this thread.
     *
     *  @return the collector assigned to this thread.
     */
    ESTFResultCollector *getCollector();

protected:

    /** Create a new thread and run the test component.
     */
    virtual void run();

private:
    ESTFComponentPtr _component;
    ESTFResultCollector *_collector;
    volatile bool _result;
};

#endif /* ! ESTF_COMPONENT_THREAD_H */
