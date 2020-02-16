#ifndef ESTF_CONCURRENCY_COMPOSITE_H
#define ESTF_CONCURRENCY_COMPOSITE_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifndef ESTF_COMPOSITE_H
#include <ESTFComposite.h>
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

/** ConcurrencyComposites contain many Components and run them
 *  concurrently.  When the its run method is called, it will create new
 *  threads to call each each child's run method in different threads of
 *  control.
 *
 *  @ingroup unit-test
 */
class ConcurrencyComposite : public Composite {
 public:
  /** Default Constructor. */
  ConcurrencyComposite();

  /** Destructor.
   */
  virtual ~ConcurrencyComposite();

  /** Run the component.  This will create new threads to run each child
   *  component in a different thread of control.
   *
   *    @param collector A result collector that will collect the results of
   *        this test run.
   *    @return true if the test run was successfully performed by the test
   *        framework.  Application errors discovered during a test run do not
   *        count, a false return means there was an error in the test suite
   *        itself that prevented it from completing one or more test cases.
   */
  virtual bool run(ResultCollector *collector);

  /** Returns a deep copy of the component.  This will return a deep copy
   *    of this composite and deep copy of every child component in the
   *    composite.
   *
   *    @return A deep copy of the composite.
   */
  virtual ComponentPtr clone();
};

ESTF_OBJECT_PTR(ConcurrencyComposite, Composite);

}  // namespace ESTF

#endif
