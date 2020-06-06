#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
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

/** FixedAllocatorTest is the unit test for FixedAllocator.
 *
 *  @ingroup foundation_test
 */
class FixedAllocatorTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  FixedAllocatorTest();

  /** Destructor. */
  virtual ~FixedAllocatorTest();

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
  int generateAllocLifetime();

  ESTF::Rand _rand;
  FixedAllocator _allocator;
};

ESTF_OBJECT_PTR(FixedAllocatorTest, ESTF::Component)

static const int Iterations = 2000;
static const int Allocations = 160;
static const bool Debug = false;
static const int Blocks = 160;
static const int BlockSize = 4096;

FixedAllocatorTest::FixedAllocatorTest() : _rand(), _allocator(Blocks, BlockSize, SystemAllocator::Instance()) {}

FixedAllocatorTest::~FixedAllocatorTest() {}

bool FixedAllocatorTest::run(ESTF::ResultCollector *collector) {
  Error error;
  char buffer[256];

  struct allocation {
    int _lifetime;
    void *_data;
  } allocations[Allocations];

  memset(allocations, 0, sizeof(allocations));

  for (int i = 0; i < Iterations; ++i) {
    for (int j = 0; j < Allocations; ++j) {
      if (allocations[j]._data && allocations[j]._lifetime == i) {
        if (Debug) {
          std::cerr << "Freeing block at time " << i << std::endl;
        }

        char *data = (char *)allocations[j]._data;

        // Make sure no one else overwrote this block.
        for (int k = 0; k < BlockSize; ++k) {
          ESTF_ASSERT(collector, (i % 127) == data[k]);
        }

        error = _allocator.deallocate(allocations[j]._data);
        allocations[j]._data = 0;

        if (ESB_SUCCESS != error) {
          DescribeError(error, buffer, sizeof(buffer));
          ESTF_FAILURE(collector, buffer);
        }

        ESTF_ASSERT(collector, ESB_SUCCESS == error);
        ESTF_ASSERT(collector, !allocations[j]._data);

        allocations[j]._lifetime = 0;
        allocations[j]._data = 0;
      }

      if (!allocations[j]._data && 1 == _rand.generateRandom(1, 4)) {
        allocations[j]._lifetime = i + generateAllocLifetime();

        if (Debug) {
          std::cerr << "Allocating block at time " << i << " with lifetime " << allocations[j]._lifetime;
        }

        allocations[j]._data = _allocator.allocate(BlockSize);

        if (0 == allocations[j]._data) {
          DescribeError(ESB_OUT_OF_MEMORY, buffer, sizeof(buffer));
          ESTF_FAILURE(collector, buffer);
        }

        ESTF_ASSERT(collector, allocations[j]._data);

        if (!allocations[j]._data) {
          if (Debug) {
            std::cerr << "Failed to allocate block at time " << i;
          }

          allocations[j]._lifetime = 0;
        } else {
          //
          // We will check this value when we free the block to
          // make sure no one overwrote it.
          //
          memset(allocations[j]._data, allocations[j]._lifetime % 127, BlockSize);
        }
      }
    }
  }

  // Simulation mostly done, return everything to the allocator.

  for (int i = 0; i < Allocations; ++i) {
    if (allocations[i]._data) {
      if (Debug) {
        std::cerr << "Freeing block at cleanup stage\n";
      }

      error = _allocator.deallocate(allocations[i]._data);
      allocations[i]._data = 0;

      if (ESB_SUCCESS != error) {
        DescribeError(error, buffer, sizeof(buffer));
        ESTF_FAILURE(collector, buffer);
      }

      ESTF_ASSERT(collector, ESB_SUCCESS == error);
      ESTF_ASSERT(collector, !allocations[i]._data);

      allocations[i]._lifetime = 0;
      allocations[i]._data = 0;
    }
  }

  return true;
}

bool FixedAllocatorTest::setup() { return true; }

bool FixedAllocatorTest::tearDown() { return true; }

ESTF::ComponentPtr FixedAllocatorTest::clone() {
  ESTF::ComponentPtr component(new FixedAllocatorTest());

  return component;
}

int FixedAllocatorTest::generateAllocLifetime() {
  int uniformDeviate = _rand.generateRandom(1, 3);

  switch (uniformDeviate) {
    case 1:
      return _rand.generateRandom(1, 10);

    case 2:
      return _rand.generateRandom(1, 100);

    case 3:
      return _rand.generateRandom(1, 1000);
  }

  return 10;
}

}  // namespace ESB

int main() {
  ESB::FixedAllocatorTestPtr fixedAllocatorTest = new ESB::FixedAllocatorTest();

  ESTF::ConcurrencyDecoratorPtr fixedAllocatorDecorator = new ESTF::ConcurrencyDecorator(fixedAllocatorTest, 3);

  ESTF::CompositePtr testSuite = new ESTF::Composite();

  testSuite->add(fixedAllocatorDecorator);

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
