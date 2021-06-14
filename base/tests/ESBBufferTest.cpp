#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESB_BUFFER_H
#include <ESBBuffer.h>
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

static const int BufferSize = 4096;
static const int Iterations = 100;

/** BufferTest is the unit test for Buffer.
 *
 *  @ingroup foundation_test
 */
class BufferTest : public ESTF::Component {
 public:
  /** Constructor.
   */
  BufferTest();

  /** Destructor. */
  virtual ~BufferTest();

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
  void fillTest(ESTF::ResultCollector *collector, unsigned int startPosition, unsigned int endPosition);

  void drainTest(ESTF::ResultCollector *collector, unsigned int startPosition, unsigned int endPosition);

  unsigned int _capacity;
  ESTF::Rand _rand;
  Buffer *_buffer;
};

ESTF_OBJECT_PTR(BufferTest, ESTF::Component)

BufferTest::BufferTest()
    : _capacity(BufferSize), _rand(), _buffer(Buffer::Create(SystemAllocator::Instance(), BufferSize)) {}

BufferTest::~BufferTest() {
  if (_buffer) {
    Buffer::Destroy(SystemAllocator::Instance(), _buffer);
  }
}

void BufferTest::fillTest(ESTF::ResultCollector *collector, unsigned int startPosition, unsigned int endPosition) {
  _buffer->setWritePosition(startPosition);

  ESTF_ASSERT(collector, _capacity == _buffer->capacity());
  ESTF_ASSERT(collector, _capacity - startPosition == _buffer->writable());

  if (0 < _capacity - startPosition) {
    ESTF_ASSERT(collector, true == _buffer->isWritable());
  } else {
    ESTF_ASSERT(collector, false == _buffer->isWritable());
  }

  for (unsigned int i = startPosition; i < endPosition; ++i) {
    ESTF_ASSERT(collector, true == _buffer->isWritable());

    _buffer->putNext('a');

    ESTF_ASSERT(collector, _capacity == _buffer->capacity());
    ESTF_ASSERT(collector, _capacity - i - 1 == _buffer->writable());
    ESTF_ASSERT(collector, i + 1 == _buffer->writePosition());
  }

  ESTF_ASSERT(collector, _capacity == _buffer->capacity());
  ESTF_ASSERT(collector, _capacity - endPosition == _buffer->writable());
  ESTF_ASSERT(collector, endPosition == _buffer->writePosition());

  if (0 < _capacity - endPosition) {
    ESTF_ASSERT(collector, true == _buffer->isWritable());
  } else {
    ESTF_ASSERT(collector, false == _buffer->isWritable());
  }
}

void BufferTest::drainTest(ESTF::ResultCollector *collector, unsigned int startPosition, unsigned int endPosition) {
  _buffer->setReadPosition(startPosition);

  ESTF_ASSERT(collector, _capacity == _buffer->capacity());
  ESTF_ASSERT(collector, endPosition - startPosition == _buffer->readable());

  if (0 < endPosition - startPosition) {
    ESTF_ASSERT(collector, true == _buffer->isReadable());
  } else {
    ESTF_ASSERT(collector, false == _buffer->isReadable());
  }

  for (unsigned int i = startPosition; i < endPosition; ++i) {
    ESTF_ASSERT(collector, true == _buffer->isReadable());

    ESTF_ASSERT(collector, 'a' == _buffer->next());

    ESTF_ASSERT(collector, _capacity == _buffer->capacity());
    ESTF_ASSERT(collector, endPosition - i - 1 == _buffer->readable());
    ESTF_ASSERT(collector, i + 1 == _buffer->readPosition());
  }

  ESTF_ASSERT(collector, _capacity == _buffer->capacity());
  ESTF_ASSERT(collector, 0 == _buffer->readable());
  ESTF_ASSERT(collector, false == _buffer->isReadable());
}

bool BufferTest::run(ESTF::ResultCollector *collector) {
  fillTest(collector, 0, _capacity);

  drainTest(collector, 0, _capacity);

  _buffer->clear();

  int startPosition = 0;
  int endPosition = _capacity;

  for (int i = 0; i < Iterations; ++i) {
    endPosition = _rand.generateRandom(2, _capacity);
    startPosition = _rand.generateRandom(0, endPosition - 1);

    fillTest(collector, startPosition, endPosition);

    drainTest(collector, startPosition, endPosition);

    _buffer->compact();
  }

  return true;
}

bool BufferTest::setup() { return true; }

bool BufferTest::tearDown() { return true; }

ESTF::ComponentPtr BufferTest::clone() {
  ESTF::ComponentPtr component(new BufferTest());

  return component;
}

}  // namespace ESB

int main() {
  ESB::BufferTestPtr bufferTest = new ESB::BufferTest();

  ESTF::ConcurrencyDecoratorPtr concurrencyDecorator = new ESTF::ConcurrencyDecorator(bufferTest, 3);

  ESTF::CompositePtr composite = new ESTF::Composite();

  composite->add(concurrencyDecorator);

  ESTF::RepetitionDecoratorPtr root = new ESTF::RepetitionDecorator(composite, 3);

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
