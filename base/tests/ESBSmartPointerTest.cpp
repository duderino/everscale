#ifndef ESB_SMART_PTR_H
#include <ESBSmartPointer.h>
#endif

#ifndef ESB_SMART_POINTER_DEBUGGER_H
#include <ESBSmartPointerDebugger.h>
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

#ifndef ESB_REFERENCE_COUNT_H
#include <ESBReferenceCount.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
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

#ifndef ESB_FIXED_ALLOCATOR_H
#include <ESBFixedAllocator.h>
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

/** SmartPointerTest is the unit test for SmartPointer
 *
 *  @ingroup foundation_test
 */
class SmartPointerTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  SmartPointerTest();

  /** Destructor. */
  virtual ~SmartPointerTest();

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
};

ESTF_OBJECT_PTR(SmartPointerTest, ESTF::Component)

class ReferenceCountSubclass : public ReferenceCount {
 public:
  ReferenceCountSubclass() : _theNumber(12) {}

  ~ReferenceCountSubclass() {}

  int getNumber() { return _theNumber; }

 private:
  int _theNumber;
};

ESB_SMART_POINTER(ReferenceCountSubclass, ReferenceCountSubclassPointer,
                  SmartPointer);

SmartPointerTest::SmartPointerTest() {}

SmartPointerTest::~SmartPointerTest() {}

bool SmartPointerTest::run(ESTF::ResultCollector *collector) {
  SmartPointerDebugger *debugger = SmartPointerDebugger::Instance();

  ESTF::Rand generator;

  Allocator *source = SystemAllocator::GetInstance();

  ReferenceCountSubclassPointer *array =
      (ReferenceCountSubclassPointer *)source->allocate(
          sizeof(ReferenceCountSubclassPointer) * 1000);

  FixedAllocator allocator(1000, sizeof(ReferenceCountSubclass), source);

  int initialRefs = debugger->getSize();

  int activeRefs = initialRefs;

  int randIdx = 0;

  for (int i = 0; i < 10000; ++i) {
    randIdx = generator.generateRandom(0, 999);

    // Each iteration, 1/4 chance to delete, else, create.

    if (1 == generator.generateRandom(1, 4)) {
      if (array[randIdx].isNull()) {
        continue;
      }

      --activeRefs;

      array[randIdx].setNull();

      ESTF_ASSERT(collector, activeRefs == debugger->getSize());

      continue;
    }

    if (array[randIdx].isNull()) {
      ++activeRefs;
    }

    // Alternate between the fixed length allocator and the system allocator
    // The smart pointer will keep track.

    if (1 == generator.generateRandom(1, 2)) {
      array[randIdx] = new (&allocator) ReferenceCountSubclass();
    } else {
      array[randIdx] = new ReferenceCountSubclass();
    }

    ESTF_ASSERT(collector, activeRefs == debugger->getSize());

    ESTF_ASSERT(collector, 12 == array[randIdx]->getNumber());
  }

  ESTF_ASSERT(collector, activeRefs == debugger->getSize());

  for (int i = 0; i < 1000; ++i) {
    array[i].setNull();
  }

  ESTF_ASSERT(collector, initialRefs == debugger->getSize());

  return true;
}

bool SmartPointerTest::setup() { return true; }

bool SmartPointerTest::tearDown() { return true; }

ESTF::ComponentPtr SmartPointerTest::clone() {
  ESTF::ComponentPtr component(new SmartPointerTest());

  return component;
}

}  // namespace ESB

int main() {
  ESB::SmartPointerDebugger::Initialize();

  ESB::SmartPointerTestPtr objectPtrTest = new ESB::SmartPointerTest();

  ESTF::CompositePtr root = new ESTF::Composite();

  root->add(objectPtrTest);

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
  std::cout << "Remaining ESBObject references: "
            << ESB::SmartPointerDebugger::Instance()->getSize() << std::endl;

  ESB::SmartPointerDebugger::Destroy();

  return collector.getStatus();
}
