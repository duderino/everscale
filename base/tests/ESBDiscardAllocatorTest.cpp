#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

#ifndef ESTF_CONCURRENCY_DECORATOR_H
#include <ESTFConcurrencyDecorator.h>
#endif

#ifndef ESTF_REPETITION_DECORATOR_H
#include <ESTFRepetitionDecorator.h>
#endif

#ifndef ESTF_RESULT_COLLECTOR_H
#include <ESTF::ResultCollector.h>
#endif

#ifndef ESTF_COMPOSITE_H
#include <ESTFComposite.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ESB_THREAD_H
#include <ESBThread.h>
#endif

#ifndef ESB_TIME_H
#include <ESBTime.h>
#endif

namespace ESB {

/** DiscardAllocatorTest is the unit test for DiscardAllocator.
 *
 *  @ingroup foundation_test
 */
class DiscardAllocatorTest : public ESTF::Component {
 public:
  /** Constructor.
   */
  DiscardAllocatorTest();

  /** Destructor. */
  virtual ~DiscardAllocatorTest();

  /** Run the component.
   *
   *  @param collector A result collector that will collect the results of
   *      this test run.
   *  @return true if the test run was successfully performed by the test
   *      framework.  Application errors discovered during a test run do not
   *      count, a false return means there was an error in the test suite
   *      itself that prevented it from completing one or more test cases.
   */
  bool run(ESTF::ResultCollector *collector);

  /** Perform a one-time initialization of the component.  Initializations
   *  that must be performed on every run of a test case should be put in
   *  the run method.
   *
   *  @return true if the one-time initialization was successfully performed,
   *      false otherwise.
   */
  bool setup();

  /** Perform a one-time tear down of the component.  Tear downs that must be
   *  performed on every run of a test case should be put in the run method.
   *
   *  @return true if the one-time tear down was successfully performed,
   *      false otherwise.
   */
  bool tearDown();

  /** Returns a deep copy of the component.
   *
   *  @return A deep copy of the component.
   */
  ESTF::ComponentPtr clone();

 private:
  int generateAllocSize();

  ESTF::Rand _rand;
  DiscardAllocator _allocator;
};

ESTF_OBJECT_PTR(DiscardAllocatorTest, ESTF::Component)

static const int ChunkSize = 1024;
static const int Iterations = 10;
static const int AllocationsPerIteration = 1000;

DiscardAllocatorTest::DiscardAllocatorTest()
    : _rand(Time::Instance().now().microSeconds() + Thread::CurrentThreadId()),
      _allocator(ChunkSize, ESB_CACHE_LINE_SIZE, 1, SystemAllocator::Instance(), true) {}

DiscardAllocatorTest::~DiscardAllocatorTest() {}

bool DiscardAllocatorTest::run(ESTF::ResultCollector *collector) {
  Error error;
  char buffer[256];
  char *data = NULL;
  int allocSize = 0;

  for (int i = 0; i < Iterations; ++i) {
    for (int j = 0; j < AllocationsPerIteration; ++j) {
      allocSize = generateAllocSize();
      error = _allocator.allocate(allocSize, (void **)&data);
      if (ESB_SUCCESS != error) {
        DescribeError(error, buffer, sizeof(buffer));
        ESTF_FAILURE(collector, buffer);
      }
      ESTF_ASSERT(collector, data);

      for (int k = 0; k < allocSize; ++k) {
        data[k] = 'b';
      }
    }

    error = _allocator.reset();

    if (ESB_SUCCESS != error) {
      DescribeError(error, buffer, sizeof(buffer));
      ESTF_FAILURE(collector, buffer);
      ESTF_ERROR(collector, "Failed to reset allocator");
      return false;
    }
  }

  return true;
}

bool DiscardAllocatorTest::setup() { return true; }

bool DiscardAllocatorTest::tearDown() { return true; }

ESTF::ComponentPtr DiscardAllocatorTest::clone() {
  ESTF::ComponentPtr component(new DiscardAllocatorTest());

  return component;
}

int DiscardAllocatorTest::generateAllocSize() {
  int uniformDeviate = _rand.generateRandom(1, 10);

  // 80% of allocations are less than 1/10 the chunksize

  if (9 > uniformDeviate) {
    return _rand.generateRandom(1, ChunkSize / 10);
  }

  // 10% of allocations are between 1/10 and the chunksize

  if (9 == uniformDeviate) {
    return _rand.generateRandom(ChunkSize / 10, ChunkSize);
  }

  // 10% exceed the chunksize

  return _rand.generateRandom(ChunkSize, ChunkSize * 2);
}

}  // namespace ESB

int main() {
  ESB::DiscardAllocatorTestPtr discardAllocatorTest = new ESB::DiscardAllocatorTest();

  ESTF::ConcurrencyDecoratorPtr discardAllocatorDecorator = new ESTF::ConcurrencyDecorator(discardAllocatorTest, 3);

  ESTF::CompositePtr testSuite = new ESTF::Composite();

  testSuite->add(discardAllocatorDecorator);

  ESTF::RepetitionDecoratorPtr root = new ESTF::RepetitionDecorator(testSuite, 3);

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
