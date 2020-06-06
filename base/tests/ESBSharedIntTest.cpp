#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
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

/** SharedIntTest is the unit test for the SharedInt class.
 *
 *  @ingroup foundation_test
 */
class SharedIntTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  SharedIntTest();

  /** Destructor. */
  virtual ~SharedIntTest();

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
  inline int getInt() { return _Int.get(); }

  inline int getUnprotectedInt() { return _UnprotectedInt; }

#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  __attribute__((no_sanitize("thread")))
#endif
#endif
  inline static void
  AddUnprotectedInt(int value) {
    _UnprotectedInt += value;
  }

#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
  __attribute__((no_sanitize("thread")))
#endif
#endif
  inline static void
  SubtractUnprotectedInt(int value) {
    _UnprotectedInt -= value;
  }

 private:
  static SharedInt _Int;
  static int _UnprotectedInt;
};

ESTF_OBJECT_PTR(SharedIntTest, ESTF::Component)

SharedInt SharedIntTest::_Int;
int SharedIntTest::_UnprotectedInt;

SharedIntTest::SharedIntTest() {}

SharedIntTest::~SharedIntTest() {}

bool SharedIntTest::run(ESTF::ResultCollector *collector) {
  int value = 0;

  for (int i = 0; i < 100000; ++i) {
    _Int.add(1);
    AddUnprotectedInt(1);

    value = _Int.inc();
    AddUnprotectedInt(1);
  }

  fprintf(stderr, "Value: %d, Shared counter: %d, unprotected counter %d\n", value, _Int.get(), _UnprotectedInt);

  for (int i = 0; i < 100000; ++i) {
    _Int.sub(1);
    SubtractUnprotectedInt(1);

    value = _Int.dec();
    SubtractUnprotectedInt(1);
  }

  fprintf(stderr, "Value: %d, Shared counter: %d, unprotected counter %d\n", value, _Int.get(), _UnprotectedInt);

  return true;
}

bool SharedIntTest::setup() { return true; }

bool SharedIntTest::tearDown() { return true; }

ESTF::ComponentPtr SharedIntTest::clone() {
  ESTF::ComponentPtr component(this);
  return component;
}

}  // namespace ESB

int main() {
  ESB::SharedIntTestPtr sharedIntTest = new ESB::SharedIntTest();
  ESTF::ConcurrencyDecoratorPtr sharedIntDecorator = new ESTF::ConcurrencyDecorator(sharedIntTest, 100);
  ESTF::CompositePtr testSuite = new ESTF::Composite();
  ESTF::RepetitionDecoratorPtr root = new ESTF::RepetitionDecorator(testSuite, 1);
  ESTF::ResultCollector collector;

  testSuite->add(sharedIntDecorator);

  if (false == root->setup()) {
    std::cerr << "Testing framework setup failed" << std::endl;
    return 1;
  }

  if (false == root->run(&collector)) {
    std::cerr << "Testing framework run failed" << std::endl;
    return 1;
  }

  ESTF_ASSERT((&collector), 0 == sharedIntTest->getInt());

  if (false == root->tearDown()) {
    std::cerr << "Testing framework tear down failed" << std::endl;
    return 1;
  }

  std::cout << collector << std::endl;

  return collector.getStatus();
}
