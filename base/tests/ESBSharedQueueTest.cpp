#ifndef ESB_SHARED_QUEUE_H
#include <ESBSharedQueue.h>
#endif

#ifndef ESB_SHARED_QUEUE_PRODUCER_H
#include <ESBSharedQueueProducer.h>
#endif

#ifndef ESB_SHARED_QUEUE_CONSUMER_H
#include <ESBSharedQueueConsumer.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESTF_CONCURRENCY_COMPOSITE_H
#include <ESTFConcurrencyComposite.h>
#endif

#ifndef ESTF_CONCURRENCY_DECORATOR_H
#include <ESTFConcurrencyDecorator.h>
#endif

#ifndef ESTF_REPETITION_DECORATOR_H
#include <ESTFRepetitionDecorator.h>
#endif

#ifndef ESTF_COMPOSITE_H
#include <ESTFComposite.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

namespace ESB {

/** SharedQueueTest is part of the unit test for SharedQueue.
 *
 *  @ingroup foundation_test
 */
class SharedQueueTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  SharedQueueTest();

  /** Destructor. */
  virtual ~SharedQueueTest();

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
};

ESTF_OBJECT_PTR(SharedQueueTest, ESTF::Component)

SharedQueueTest::SharedQueueTest() {}

SharedQueueTest::~SharedQueueTest() {}

bool SharedQueueTest::run(ESTF::ResultCollector *collector) {
  ESTF::ComponentPtr component;
  SharedQueue queue(SystemAllocator::GetInstance(), 20);
  ESTF::ConcurrencyComposite composite;

  for (int i = 0; i < 9; ++i) {
    component = new SharedQueueProducer(i, queue, ESB_UINT32_C(10000));

    composite.add(component);
  }

  for (int i = 0; i < 3; ++i) {
    component = new SharedQueueConsumer(queue, ESB_UINT32_C(30000));

    composite.add(component);
  }

  bool result = composite.run(collector);

  composite.clear();

  return result;
}

bool SharedQueueTest::setup() { return true; }

bool SharedQueueTest::tearDown() { return true; }

ESTF::ComponentPtr SharedQueueTest::clone() {
  ESTF::ComponentPtr component(new SharedQueueTest());

  return component;
}

}  // namespace ESB

int main() {
  ESB::SharedQueueTestPtr sharedQueueTest = new ESB::SharedQueueTest();

  ESTF::ConcurrencyDecoratorPtr sharedQueueDecorator =
      new ESTF::ConcurrencyDecorator(sharedQueueTest, 3);

  ESTF::CompositePtr testSuite = new ESTF::Composite();

  testSuite->add(sharedQueueDecorator);

  ESTF::RepetitionDecoratorPtr root =
      new ESTF::RepetitionDecorator(testSuite, 3);

  ESTF::ResultCollector collector;

  if (false == root->setup()) {
    std::cerr << "Testing framework setup failed" << std::endl;
    return 1;
  }

  if (false == root->run(&collector)) {
    std::cerr << "Testing framework run failed" << std::endl;
    return 1;
  }

  if (false == root->tearDown()) {
    std::cerr << "Testing framework tear down failed" << std::endl;
    return 1;
  }

  std::cout << collector << std::endl;

  return collector.getStatus();
}
