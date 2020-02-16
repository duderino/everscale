/** @file ESTFComponent.h
 *  @brief A generic interface for all test components in the test suite.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 */

#ifndef ESTF_COMPONENT_H
#define ESTF_COMPONENT_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_H
#include <ESTFObject.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

namespace ESTF {

class ComponentPtr;

/**    Component defines the interface for any test component in the 
 *  test frameork.  Individual concrete test cases, test suites, and a variety
 *  of decorators all realize this interface.
 *
 *  @ingroup test
 */
class Component : public Object
{
public:
    /** Constructor. */
    Component() : Object() {};

    /** Destructor. */
    virtual ~Component() {};

    /** Run the component.
     *
     *    @param collector A result collector that will collect the results of
     *        this test run.
     *    @return true if the test run was successfully performed by the test
     *        framework.  Application errors discovered during a test run do not
     *        count, a false return means there was an error in the test suite
     *        itself that prevented it from completing one or more test cases.
     */
    virtual bool run( ResultCollector *collector ) = 0;

    /** Perform a one-time initialization of the component.  Initializations
     *    that must be performed on every run of a test case should be put in
     *    the run method.
     *
     *    @return true if the one-time initialization was successfully performed,
     *        false otherwise.
     */
    virtual bool setup() = 0;

    /** Perform a one-time tear down of the component.  Tear downs that must be
     *    performed on every run of a test case should be put in the run method.
     *
     *    @return true if the one-time tear down was successfully performed,
     *        false otherwise.
     */
    virtual bool tearDown() = 0;

    /** Returns a deep copy of the component.
     *
     *    @return A deep copy of the component.
     */
    virtual ComponentPtr clone() = 0;
};

ESTF_OBJECT_PTR(Component,Object)

}

#endif
