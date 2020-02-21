#ifndef ESB_BUDDY_ALLOCATOR_H
#include <ESBBuddyAllocator.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
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

namespace ESB {

static const int Allocations = 160;
static const bool Debug = false;
static const int Iterations = 2000;

/** BuddyAllocatorTest is the unit test for BuddyAllocator.
 *
 *  @ingroup base_test
 */
class BuddyAllocatorTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  BuddyAllocatorTest();

  /** Destructor. */
  virtual ~BuddyAllocatorTest();

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
  int generateAllocSize();
  int generateAllocLifetime();

  ESTF::Rand _rand;
  BuddyAllocator _allocator;
};

ESTF_OBJECT_PTR(BuddyAllocatorTest, ESTF::Component)

BuddyAllocatorTest::BuddyAllocatorTest()
    : _rand(), _allocator(17, SystemAllocator::GetInstance()) {}

BuddyAllocatorTest::~BuddyAllocatorTest() {}

bool BuddyAllocatorTest::run(ESTF::ResultCollector *collector) {
  //
  //  This test case follows Knuth's test case for the Buddy System ( and
  //  other allocators ) in The Art of Computer Programming, Volume 1
  //  Fundamental Algorithms Third Edition, p. 146.
  //

  Error error;
  int bytesAllocated = 0;
  char buffer[256];

  error = _allocator.initialize();

  if (ESB_SUCCESS != error) {
    DescribeError(error, buffer, sizeof(buffer));

    ESTF_FAILURE(collector, buffer);
    ESTF_ERROR(collector, "Failed to initialize allocator");

    return false;
  }

  struct allocation {
    int size;
    int lifetime;
    void *data;
  } allocations[Allocations];

  memset(allocations, 0, sizeof(allocations));

  for (int i = 0; i < Iterations; ++i) {
    for (int j = 0; j < Allocations; ++j) {
      if (0 < allocations[j].size && allocations[j].lifetime == i) {
        if (Debug) {
          std::cerr << "Freeing block of size " << allocations[j].size
                    << " at time " << i << std::endl;
        }

        char *data = (char *)allocations[j].data;

        // Make sure no one else overwrote this block.
        for (int k = 0; k < allocations[j].size; ++k) {
          ESTF_ASSERT(collector, (i % 127) == data[k]);
        }

        error = _allocator.deallocate(allocations[j].data);
        allocations[j].data = 0;

        if (ESB_SUCCESS != error) {
          DescribeError(error, buffer, sizeof(buffer));
          ESTF_FAILURE(collector, buffer);
        }

        ESTF_ASSERT(collector, ESB_SUCCESS == error);
        ESTF_ASSERT(collector, !allocations[j].data);

        bytesAllocated -= allocations[j].size;

        allocations[j].size = 0;
        allocations[j].lifetime = 0;
        allocations[j].data = 0;
      }

      if (0 == allocations[j].size && 1 == _rand.generateRandom(1, 4)) {
        ESTF_ASSERT(collector, 0 == allocations[j].lifetime);
        ESTF_ASSERT(collector, 0 == allocations[j].data);

        allocations[j].size = generateAllocSize();
        allocations[j].lifetime = i + generateAllocLifetime();

        if (Debug) {
          std::cerr << "Allocating block of size " << allocations[j].size
                    << " at time " << i
                    << " with lifetime: " << allocations[j].lifetime
                    << std::endl;
        }

        error = _allocator.allocate(&allocations[j].data, allocations[j].size);

        //
        //  We allow failures after half of the allocators memory
        //  has been allocated.
        //
        if ((bytesAllocated < (1 << 16)) || allocations[j].data) {
          if (ESB_SUCCESS != error) {
            DescribeError(error, buffer, sizeof(buffer));
            ESTF_FAILURE(collector, buffer);
          }

          ESTF_ASSERT(collector, ESB_SUCCESS == error);
          ESTF_ASSERT(collector, allocations[j].data);
        } else {
          if (ESB_OUT_OF_MEMORY != error) {
            DescribeError(error, buffer, sizeof(buffer));
            ESTF_FAILURE(collector, buffer);
          }

          ESTF_ASSERT(collector, ESB_OUT_OF_MEMORY == error);
        }

        if (!allocations[j].data) {
          if (Debug) {
            std::cerr << "Failed to allocate block of size "
                      << allocations[j].size << " at time " << i
                      << " with lifetime: " << allocations[j].lifetime
                      << std::endl;
          }

          allocations[j].lifetime = 0;
          allocations[j].size = 0;
        } else {
          //
          // We will check this value when we free the block to
          // make sure no one overwrote it.
          //
          memset(allocations[j].data, allocations[j].lifetime % 127,
                 allocations[j].size);

          bytesAllocated += allocations[j].size;
        }

        if (Debug) {
          std::cerr << bytesAllocated << " out of " << (1 << 17)
                    << " bytes allocated\n";
        }
      }

      if (Debug) {
        int size = 0;

        for (int k = 0; k < Allocations; ++k) {
          size += allocations[k].size;
        }

        ESTF_ASSERT(collector, size == bytesAllocated);
      }
    }
  }

  // Simulation mostly done, return everything to the allocator.

  for (int i = 0; i < Allocations; ++i) {
    if (0 < allocations[i].size) {
      if (Debug) {
        std::cerr << "Freeing block of size " << allocations[i].size
                  << " at cleanup stage\n";
      }

      error = _allocator.deallocate(allocations[i].data);
      allocations[i].data = 0;

      if (ESB_SUCCESS != error) {
        DescribeError(error, buffer, sizeof(buffer));
        ESTF_FAILURE(collector, buffer);
      }

      ESTF_ASSERT(collector, ESB_SUCCESS == error);
      ESTF_ASSERT(collector, !allocations[i].data);

      allocations[i].size = 0;
      allocations[i].lifetime = 0;
      allocations[i].data = 0;
    }
  }

  //
  // The buddy allocator should have coalesced everything back into one
  // big block, try a really big allocation.
  //

  error = _allocator.allocate(&allocations[0].data, 1 << 16);

  if (ESB_SUCCESS != error) {
    DescribeError(error, buffer, sizeof(buffer));
    ESTF_FAILURE(collector, buffer);
  }

  ESTF_ASSERT(collector, ESB_SUCCESS == error);
  ESTF_ASSERT(collector, allocations[0].data);

  if (allocations[0].data) {
    memset(allocations[0].data, 'B', 1 << 16);

    error = _allocator.destroy();

    ESTF_ASSERT(collector, ESB_IN_USE == error);

    if (Debug) {
      std::cerr << "Allocated big block of size " << (1 << 16) << std::endl;
    }

    error = _allocator.deallocate(allocations[0].data);
    allocations[0].data = 0;

    if (ESB_SUCCESS != error) {
      DescribeError(error, buffer, sizeof(buffer));
      ESTF_FAILURE(collector, buffer);
    }

    ESTF_ASSERT(collector, ESB_SUCCESS == error);
    ESTF_ASSERT(collector, !allocations[0].data);
  } else {
    std::cerr << "Failed to alloc big block of size " << (1 << 16) << std::endl;
  }

  error = _allocator.destroy();

  if (ESB_SUCCESS != error) {
    DescribeError(error, buffer, sizeof(buffer));
    ESTF_FAILURE(collector, buffer);
    ESTF_ERROR(collector, "Failed to destroy allocator");
    return false;
  }

  ESTF_ASSERT(collector, ESB_SUCCESS == error);

  return true;
}

bool BuddyAllocatorTest::setup() { return true; }

bool BuddyAllocatorTest::tearDown() { return true; }

ESTF::ComponentPtr BuddyAllocatorTest::clone() {
  ESTF::ComponentPtr component(new BuddyAllocatorTest());

  return component;
}

int BuddyAllocatorTest::generateAllocSize() {
  static const int array1[32] = {1, 1, 1, 1, 1, 1, 1,  1,  1,  1, 1,
                                 1, 1, 1, 1, 1, 2, 2,  2,  2,  2, 2,
                                 2, 2, 4, 4, 4, 4, 16, 16, 32, 32};

  static const int array2[22] = {10,  12,  14,   16,   18,   20,  30,  40,
                                 50,  60,  70,   80,   90,   100, 150, 200,
                                 250, 500, 1000, 2000, 3000, 4000};

  int uniformDeviate = _rand.generateRandom(1, 3);

  switch (uniformDeviate) {
    case 1:
      return _rand.generateRandom(100, 2000);

    case 2:
      return array1[_rand.generateRandom(0, 31)];

    case 3:
      return array2[_rand.generateRandom(0, 21)];
  }

  return 10;
}

int BuddyAllocatorTest::generateAllocLifetime() {
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
  ESB::BuddyAllocatorTestPtr buddyAllocatorTest = new ESB::BuddyAllocatorTest();

  ESTF::ConcurrencyDecoratorPtr buddyAllocatorDecorator =
      new ESTF::ConcurrencyDecorator(buddyAllocatorTest, 3);

  ESTF::CompositePtr testSuite = new ESTF::Composite();

  testSuite->add(buddyAllocatorDecorator);

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
