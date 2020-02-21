#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESB_LIST_H
#include <ESBList.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
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

#ifndef ESTF_RESULT_COLLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_COMPOSITE_H
#include <ESTFComposite.h>
#endif

#include <list>

namespace ESB {

/**	ListTest is the unit test for List.
 *
 *  @ingroup foundation_test
 */
class ListTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  ListTest();

  /** Destructor. */
  virtual ~ListTest();

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
  class Record {
   public:
    Record() : _value(0), _lifetime(0), _useIterator(true), _iterator() {}

    virtual ~Record() {}

    char *_value;
    int _lifetime;
    bool _useIterator;
    ListIterator _iterator;
  };

  typedef std::list<char *>::iterator STLListIterator;

  char *generateValue(int i, int j);
  int generateLifetime();
  ListIterator findIterator(void *value);
  bool findSTLIterator(void *value, STLListIterator *);
  void validateList(ESTF::ResultCollector *collector);

  static const int _Iterations;
  static const int _Records;
  static NullLock _Lock;

  Record *_records;
  ESTF::Rand _rand;
  List _list;
  std::list<char *> _stlList;
};

ESTF_OBJECT_PTR(ListTest, ESTF::Component)

NullLock ListTest::_Lock;
const int ListTest::_Iterations = 500;
const int ListTest::_Records = 1000;
static const bool Debug = false;

ListTest::ListTest()
    : _records(0),
      _rand(),
      _list(SystemAllocator::GetInstance(), &_Lock),
      _stlList() {}

ListTest::~ListTest() {}

bool ListTest::run(ESTF::ResultCollector *collector) {
  bool stlResult = false;
  ListIterator iterator;
  STLListIterator stlIterator;

  _records = new Record[_Records];

  if (!_records) {
    ESTF_ERROR(collector, "Couldn't allocate memory");
    return false;
  }

  for (int k = 0; k < 3; ++k) {
    for (int i = 0; i < _Records; ++i) {
      _records[i]._value = 0;
      _records[i]._lifetime = 0;
      _records[i]._useIterator = false;
    }

    for (int i = 0; i < _Iterations; ++i) {
      for (int j = 0; j < _Records; ++j) {
        if (!_records[j]._value && 1 == _rand.generateRandom(1, 200)) {
          //
          // Create and insert a new node.
          //

          _records[j]._value = generateValue(i, j);
          _records[j]._lifetime = i + generateLifetime();
          _records[j]._useIterator = (1 == _rand.generateRandom(1, 2));

          if (1 == _rand.generateRandom(1, 2)) {
            _stlList.push_back(_records[j]._value);

            ESTF_ASSERT(collector,
                        ESB_SUCCESS == _list.pushBack(_records[j]._value));

            if (_records[j]._useIterator) {
              _records[j]._iterator = _list.getBackIterator();

              ESTF_ASSERT(collector, !_records[j]._iterator.isNull());
            }
          } else {
            _stlList.push_front(_records[j]._value);

            ESTF_ASSERT(collector,
                        ESB_SUCCESS == _list.pushFront(_records[j]._value));

            if (_records[j]._useIterator) {
              _records[j]._iterator = _list.getFrontIterator();

              ESTF_ASSERT(collector, !_records[j]._iterator.isNull());
            }
          }

          if (Debug) {
            std::cerr << "Inserted: " << (char *)_records[j]._value
                      << " (List size: " << _list.getSize()
                      << " stl size: " << _stlList.size() << ") at time " << i
                      << std::endl;
          }

          validateList(collector);
        }

        if (_records[j]._value && i == _records[j]._lifetime) {
          //
          //  Erase and delete an existing node.
          //

          if (_records[j]._useIterator) {
            ESTF_ASSERT(collector,
                        ESB_SUCCESS == _list.erase(&_records[j]._iterator));

            ESTF_ASSERT(collector, _records[j]._iterator.isNull());
          } else {
            iterator = findIterator(_records[j]._value);

            ESTF_ASSERT(collector, !iterator.isNull());

            ESTF_ASSERT(collector, ESB_SUCCESS == _list.erase(&iterator));

            ESTF_ASSERT(collector, iterator.isNull());
          }

          if (Debug) {
            std::cerr << "value[" << j << "]: " << _records[j]._value
                      << std::endl;
          }

          stlResult = findSTLIterator(_records[j]._value, &stlIterator);

          assert(stlResult);

          _stlList.erase(stlIterator);

          if (Debug) {
            std::cerr << "Deleted: " << (char *)_records[j]._value
                      << " (list size: " << _list.getSize()
                      << " stl size: " << _stlList.size() << ") at time " << i
                      << std::endl;
          }

          delete[] _records[j]._value;
          _records[j]._value = 0;

          validateList(collector);
        }
      }
    }

    //
    //  Cleanup
    //

    ListIterator temp;
    char *value = 0;

    for (value = (char *)_list.getFront(); !_list.isEmpty();
         value = (char *)_list.getFront()) {
      ESTF_ASSERT(collector, ESB_SUCCESS == _list.popFront());

      stlResult = findSTLIterator(value, &stlIterator);

      assert(stlResult);

      _stlList.erase(stlIterator);

      if (Debug) {
        std::cerr << "Deleted: " << value << " (List size: " << _list.getSize()
                  << " stl size: " << _stlList.size() << ") at cleanup stage"
                  << std::endl;
      }

      delete[] value;

      validateList(collector);
    }
  }

  delete[] _records;
  _records = 0;

  return true;
}

ListIterator ListTest::findIterator(void *value) {
  ListIterator it;

  for (it = _list.getFrontIterator(); it.hasNext(); it = it.getNext()) {
    if (0 == strcmp((char *)value, (char *)it.getValue())) {
      return it;
    }
  }

  return it;
}

bool ListTest::findSTLIterator(void *value, STLListIterator *it) {
  for (*it = _stlList.begin(); *it != _stlList.end(); ++(*it)) {
    if (0 == strcmp((char *)value, **it)) {
      return true;
    }
  }

  return false;
}

void ListTest::validateList(ESTF::ResultCollector *collector) {
  ListIterator iterator;
  STLListIterator stlIterator;
  char *value = 0;
  UInt32 counter = 0;

  ESTF_ASSERT(collector, _list.getSize() == _stlList.size());

  for (stlIterator = _stlList.begin(); stlIterator != _stlList.end();
       ++stlIterator) {
    iterator = findIterator(*stlIterator);

    ESTF_ASSERT(collector, !iterator.isNull());
  }

  //
  //  Forward iterate through both lists and make sure that their records
  //  are in the same order.
  //

  for (stlIterator = _stlList.begin(), iterator = _list.getFrontIterator();
       !iterator.isNull();
       ++stlIterator, iterator = iterator.getNext(), ++counter) {
    value = (char *)iterator.getValue();

    ESTF_ASSERT(collector, 0 == strcmp(value, *stlIterator));
  }

  ESTF_ASSERT(collector, !iterator.hasNext());
  ESTF_ASSERT(collector, counter == _list.getSize());

  //
  //  Reverse iterate through both lists and make sure that their records
  //  are in the same order.
  //
  counter = 0;
  stlIterator = _stlList.end();

  for (--stlIterator, iterator = _list.getBackIterator(); !iterator.isNull();
       --stlIterator, iterator = iterator.getPrevious(), ++counter) {
    value = (char *)iterator.getValue();

    ESTF_ASSERT(collector, 0 == strcmp(value, *stlIterator));
  }

  ESTF_ASSERT(collector, !iterator.hasPrevious());
  ESTF_ASSERT(collector, counter == _list.getSize());
}

bool ListTest::setup() { return true; }

bool ListTest::tearDown() { return true; }

ESTF::ComponentPtr ListTest::clone() {
  ESTF::ComponentPtr component(new ListTest());

  return component;
}

char *ListTest::generateValue(int i, int j) {
  char *value = new char[22];

  if (!value) return 0;

  sprintf(value, "%d@%d", i, j);

  return value;
}

int ListTest::generateLifetime() {
  int uniformDeviate = _rand.generateRandom(1, 3);

  switch (uniformDeviate) {
    case 1:
      return 1;

    case 2:
      return _rand.generateRandom(1, 10);

    case 3:
      return _rand.generateRandom(1, 100);
  }

  return 10;
}

}  // namespace ESB

int main() {
  ESB::ListTestPtr listTest = new ESB::ListTest();

  ESTF::ConcurrencyDecoratorPtr listDecorator =
      new ESTF::ConcurrencyDecorator(listTest, 3);

  ESTF::CompositePtr testSuite = new ESTF::Composite();

  testSuite->add(listDecorator);

  ESTF::RepetitionDecoratorPtr root =
      new ESTF::RepetitionDecorator(testSuite, 3);

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
