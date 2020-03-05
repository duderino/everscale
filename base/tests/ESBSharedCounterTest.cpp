#ifndef ESB_SHARED_COUNTER_H
#include <ESBSharedCounter.h>
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

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESTF_CONCURRENCY_DECORATOR_H
#include <ESTFConcurrencyDecorator.h>
#endif

#ifndef ESTF_REPETITION_DECORATOR_H
#include <ESTFRepetitionDecorator.h>
#endif

#ifndef ESTF_RESULT_COLLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_COMPOSITE_H
#include <ESTFComposite.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

namespace ESB {

/** SharedCounterTest is the unit test for the SharedCounter class.
 *
 *  @ingroup foundation_test
 */
class SharedCounterTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  SharedCounterTest();

  /** Destructor. */
  virtual ~SharedCounterTest();

  /** Run the component.
   *
   *	@param collector A result collector that will collect the results of
   *		this test run.
   *	@return true if the test run was successfully performed by the test
   *		framework.  Application errors discovered during a test run do
   *not count, a false return means there was an error in the test suite itself
   *that prevented it from completing one or more test cases.
   */
  bool run(ESTF::ResultCollector *collector);

  /** Perform a one-time initialization of the component.  Initializations
   *	that must be performed on every run of a test case should be put in
   *	the run method.
   *
   *	@return true if the one-time initialization was successfully performed,
   *		false otherwise.
   */
  bool setup();

  /** Perform a one-time tear down of the component.  Tear downs that must be
   *	performed on every run of a test case should be put in the run method.
   *
   *	@return true if the one-time tear down was successfully performed,
   *		false otherwise.
   */
  bool tearDown();

  /** Returns a deep copy of the component.
   *
   *	@return A deep copy of the component.
   */
  ESTF::ComponentPtr clone();

  /** Get the current state of the counter.
   *
   *	@return the counter's current count.
   */
  inline int getCounter() { return _Counter.get(); }

  inline int getUnprotectedCounter() { return _UnprotectedCounter; }

  __attribute__((no_sanitize("thread"))) inline static void
  AddUnprotectedCounter(int value) {
    _UnprotectedCounter += value;
  }

  __attribute__((no_sanitize("thread"))) inline static void
  SubtractUnprotectedCounter(int value) {
    _UnprotectedCounter -= value;
  }

 private:
  static SharedCounter _Counter;
  static int _UnprotectedCounter;
};

ESTF_OBJECT_PTR(SharedCounterTest, ESTF::Component)

SharedCounter SharedCounterTest::_Counter;
int SharedCounterTest::_UnprotectedCounter;

SharedCounterTest::SharedCounterTest() {}

SharedCounterTest::~SharedCounterTest() {}

bool SharedCounterTest::run(ESTF::ResultCollector *collector) {
  int value = 0;

  for (int i = 0; i < 100000; ++i) {
    _Counter.add(1);
    AddUnprotectedCounter(1);

    value = _Counter.inc();
    AddUnprotectedCounter(1);
  }

  fprintf(stderr, "Value: %d, Shared counter: %d, unprotected counter %d\n",
          value, _Counter.get(), _UnprotectedCounter);

  for (int i = 0; i < 100000; ++i) {
    _Counter.sub(1);
    SubtractUnprotectedCounter(1);

    value = _Counter.dec();
    SubtractUnprotectedCounter(1);
  }

  fprintf(stderr, "Value: %d, Shared counter: %d, unprotected counter %d\n",
          value, _Counter.get(), _UnprotectedCounter);

  return true;
}

bool SharedCounterTest::setup() { return true; }

bool SharedCounterTest::tearDown() { return true; }

ESTF::ComponentPtr SharedCounterTest::clone() {
  ESTF::ComponentPtr component(this);
  return component;
}

}  // namespace ESB

int main() {
  ESB::SharedCounterTestPtr sharedCounterTest = new ESB::SharedCounterTest();
  ESTF::ConcurrencyDecoratorPtr sharedCounterDecorator =
      new ESTF::ConcurrencyDecorator(sharedCounterTest, 100);
  ESTF::CompositePtr testSuite = new ESTF::Composite();
  ESTF::RepetitionDecoratorPtr root =
      new ESTF::RepetitionDecorator(testSuite, 1);
  ESTF::ResultCollector collector;

  testSuite->add(sharedCounterDecorator);

  if (false == root->setup()) {
    std::cerr << "Testing framework setup failed" << std::endl;
    return 1;
  }

  if (false == root->run(&collector)) {
    std::cerr << "Testing framework run failed" << std::endl;
    return 1;
  }

  ESTF_ASSERT((&collector), 0 == sharedCounterTest->getCounter());

  if (false == root->tearDown()) {
    std::cerr << "Testing framework tear down failed" << std::endl;
    return 1;
  }

  std::cout << collector << std::endl;

  return collector.getStatus();
}
