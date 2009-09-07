/**    @file ESTFComposite.h
 *    @brief ESTFComposites contain many ESTFComponents and run them serially.
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

#ifndef ESTF_COMPOSITE_H
#define ESTF_COMPOSITE_H

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

#include <list>

/** ESTFComposites contain many ESTFComponents and run them serially.
 *  When the composite's run method is called, the composite will iterate
 *  through its children and call each child's run method in the order that
 *  it was added.
 *
 *  @ingroup test
 */
class ESTFComposite : public ESTFComponent
{
public:
    /** Default Constructor. */
    ESTFComposite();

    /** Destructor.
     */
    virtual ~ESTFComposite();

    /** Run the component.  This will iterate through all child components in
     *    the order they were added and call each of their run methods.
     *
     *    @param collector A result collector that will collect the results of
     *        this test run.
     *    @return true if the test run was successfully performed by the test
     *        framework.  Application errors discovered during a test run do not
     *        count, a false return means there was an error in the test suite
     *        itself that prevented it from completing one or more test cases.
     */
    virtual bool run( ESTFResultCollector *collector );

    /** Perform a one-time initialization of the component.  Initializations
     *    that must be performed on every run of a test case should be put in
     *    the run method.  This will iterate through all child components in
     *    the order they were added and call each of their setup methods.
     *
     *    @return true if the one-time initialization was successfully performed,
     *        false otherwise.
     */
    virtual bool setup();

    /** Perform a one-time tear down of the component.  Tear downs that must be
     *    performed on every run of a test case should be put in the run method.
     *    This will iterate through all child components in the order they were
     *    added and call each of their tearDown methods.
     *
     *    @return true if the one-time tear down was successfully performed,
     *        false otherwise.
     */
    virtual bool tearDown();

    /** Returns a deep copy of the component.  This will return a deep copy
     *    of this composite and deep copy of every child component in the
     *    composite.
     *
     *    @return A deep copy of the composite.
     */
    virtual ESTFComponentPtr clone();

    /** Add a component to this composite.
     *
     *    @param component The component to add.
     */
    virtual void add( ESTFComponentPtr &component );

    /** Removes a component from this composite.
     *
     *    @param component The component to remove.
     */
    virtual void remove( const ESTFComponentPtr &component );

    /** Removes all components from this composite.
     *
     */
    virtual void clear();

    /** Determine the number of components contained in this composite.
     *  Warning:  This is a linear search.
     *
     *    @return The number of compoents in this composite.
     */
    virtual int size();

protected:

    std::list<ESTFComponentPtr> _children;
};

DEFINE_ESTF_OBJECT_PTR(ESTFComposite,ESTFComponent);

#endif                                 /* ! ESTF_COMPOSITE_H */
