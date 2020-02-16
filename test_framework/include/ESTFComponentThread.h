/** @file ESTFComponentThread.h
 *  @brief A thread that runs a ESTFComponent
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
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

namespace ESTF {

/** ComponentThreads run a Component in a new thread of control.
 *
 *  @ingroup test
 */
class ComponentThread : public Thread
{
public:
    /** Create a new instance */
    ComponentThread();

    /** Default destructor */
    virtual ~ComponentThread();

    /** Assign a Component to be run in this thread.
     *
     *  @param component The component to be run in this thread
     */
    void setComponent( ComponentPtr &component );

    /** Assign a collector to collect the results of the test run for this
     *  thread.
     *
     *  @param collector The collector that will collect the results of the
     *      component's test run.
     */
    void setCollector( ResultCollector *collector );

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
    ResultCollector *getCollector();

protected:

    /** Create a new thread and run the test component.
     */
    virtual void run();

private:
    ComponentPtr _component;
    ResultCollector *_collector;
    volatile bool _result;
};

}

#endif
