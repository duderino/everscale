#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESB_LOCKABLE_H
#include <ESBLockable.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESTF_THREAD_H
#include <ESTFThread.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_READ_WRITE_LOCK_H
#include <ESBReadWriteLock.h>
#endif

#ifndef ESB_COUNTING_SEMAPHORE_H
#include <ESBCountingSemaphore.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
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

namespace ESB {

/**	LockableTest is the unit test for the Lockable class.
 *
 *  @ingroup foundation_test
 */
class LockableTest : public ESTF::Component {
 public:
  /**	Constructor.
   *
   *	@param lockable The concrete Lockable to test.
   */
  LockableTest(Lockable &lockable);

  /** Destructor. */
  virtual ~LockableTest();

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

 private:
  Lockable &_lock;

  int _counter1;
  int _counter2;
};

ESTF_OBJECT_PTR(LockableTest, ESTF::Component)

LockableTest::LockableTest(Lockable &lockable)
    : ESTF::Component(), _lock(lockable), _counter1(0), _counter2(0) {}

LockableTest::~LockableTest() {}

bool LockableTest::run(ESTF::ResultCollector *collector) {
  int counter1;
  Error error;
  char buffer[256];

  for (int i = 0; i < 100; ++i) {
    error = _lock.readAcquire();

    if (ESB_SUCCESS != error) {
      DescribeError(error, buffer, sizeof(buffer));
      ESTF_FAILURE(collector, buffer);
    }

    ESTF_ASSERT(collector, ESB_SUCCESS == error);

    for (int i = 0; i < 10; ++i) {
      counter1 = _counter1;
      ESTF::Thread::Yield();
      ESTF_ASSERT(collector, counter1 == _counter2);
    }

    error = _lock.readRelease();

    if (ESB_SUCCESS != error) {
      DescribeError(error, buffer, sizeof(buffer));
      ESTF_FAILURE(collector, buffer);
    }

    error = _lock.writeAcquire();

    if (ESB_SUCCESS != error) {
      DescribeError(error, buffer, sizeof(buffer));
      ESTF_FAILURE(collector, buffer);
    }

    ++_counter1;
    ESTF::Thread::Yield();

    ++_counter2;
    ESTF_ASSERT(collector, _counter1 == _counter2);

    error = _lock.writeRelease();

    if (ESB_SUCCESS != error) {
      DescribeError(error, buffer, sizeof(buffer));
      ESTF_FAILURE(collector, buffer);
    }
  }

  return true;
}

bool LockableTest::setup() {
  _counter1 = 0;
  _counter2 = 0;

  return true;
}

bool LockableTest::tearDown() { return true; }

ESTF::ComponentPtr LockableTest::clone() {
  //
  //	Do a shallow clone so cloned instances will share the lock and the
  //	counters when they are wrapped in the concurrency decorator.
  //
  ESTF::ComponentPtr component(this);

  return component;
}

}

int main() {
  ESB::Mutex mutex;
  ESB::ReadWriteLock rwLock;
  ESB::CountingSemaphore countingSemaphore;

  countingSemaphore.writeRelease();  // Using like a binary semaphore

  ESB::LockableTestPtr mutexTest = new ESB::LockableTest(mutex);
  ESB::LockableTestPtr lockTest = new ESB::LockableTest(rwLock);
  ESB::LockableTestPtr semaphoreTest = new ESB::LockableTest(countingSemaphore);

  ESTF::ConcurrencyDecoratorPtr mutexDecorator =
      new ESTF::ConcurrencyDecorator(mutexTest, 10);
  ESTF::ConcurrencyDecoratorPtr lockDecorator =
      new ESTF::ConcurrencyDecorator(lockTest, 10);
  ESTF::ConcurrencyDecoratorPtr semaphoreDecorator =
      new ESTF::ConcurrencyDecorator(semaphoreTest, 10);

  ESTF::CompositePtr testSuite = new ESTF::Composite();

  testSuite->add(mutexDecorator);
  testSuite->add(lockDecorator);
  testSuite->add(semaphoreDecorator);

  ESTF::RepetitionDecoratorPtr root = new ESTF::RepetitionDecorator(testSuite, 3);

  ESTF::ResultCollector collector;

  if (false == root->setup()) {
    std::cerr << "Testing framework setup failed" << std::endl;
    return 1;
  }

  if (false == root->run(&collector)) {
    std::cerr << "Testing framework run failed" << std::endl;
  }

  if (false == root->tearDown()) {
    std::cerr << "Testing framework tear down failed" << std::endl;
  }

  if (0 == collector.getFailureCount() && 0 == collector.getErrorCount()) {
    std::cout << "All test cases passed" << std::endl;
  }

  std::cout << collector << std::endl;

  return 0;
}
