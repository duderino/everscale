#ifndef ESB_SHARED_QUEUE_CONSUMER_H
#define ESB_SHARED_QUEUE_CONSUMER_H

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESB_SHARED_QUEUE_H
#include <ESBSharedQueue.h>
#endif

namespace ESB {

/** SharedQueueConsumer is part of the unit test for SharedQueue.
 *
 *  @ingroup foundation_test
 */
class SharedQueueConsumer : public ESTF::Component {
 public:
  /**	Constructor.
   *
   *  @param queue The shared queue to use
   *  @param items The number of items to push into the shared queue before
   *      exiting
   */
  SharedQueueConsumer(SharedQueue &queue, UInt32 items);

  /** Destructor. */
  virtual ~SharedQueueConsumer();

  /** Run the component.
   *
   *	@param collector A result collector that will collect the results of
   *		this test run.
   *	@return true if the test run was successfully performed by the test
   *		framework.  Application errors discovered during a test run do
   *not count, a false return means there was an error in the test suite itself
   *that prevented it from completing one or more test cases.
   */
  virtual bool run(ESTF::ResultCollector *collector);

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
  virtual ESTF::ComponentPtr clone();

 private:
  UInt32 _items;
  SharedQueue &_queue;
};

ESTF_OBJECT_PTR(SharedQueueConsumer, ESTF::Component)

}

#endif
