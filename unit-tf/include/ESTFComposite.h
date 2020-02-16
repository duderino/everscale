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

namespace ESTF {

/** Composites contain many Components and run them serially.
 *  When the composite's run method is called, the composite will iterate
 *  through its children and call each child's run method in the order that
 *  it was added.
 *
 *  @ingroup unit-test
 */
class Composite : public Component
{
public:
    /** Default Constructor. */
    Composite();

    /** Destructor.
     */
    virtual ~Composite();

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
    virtual bool run( ResultCollector *collector );

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
    virtual ComponentPtr clone();

    /** Add a component to this composite.
     *
     *    @param component The component to add.
     */
    virtual void add( ComponentPtr &component );

    /** Removes a component from this composite.
     *
     *    @param component The component to remove.
     */
    virtual void remove( const ComponentPtr &component );

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

    std::list<ComponentPtr> _children;
};

ESTF_OBJECT_PTR(Composite,Component);

}

#endif
