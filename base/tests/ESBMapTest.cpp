#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

#ifndef ESB_COMPARATOR_H
#include <ESBComparator.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
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

#include <map>

namespace ESB {

/** MapTest is the unit test for Map.
 *
 *  @ingroup foundation_test
 */
class MapTest : public ESTF::Component {
 public:
  /**	Constructor.
   */
  MapTest();

  /** Destructor. */
  virtual ~MapTest();

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
    Record()
        : _key(0), _value(0), _lifetime(0), _useIterator(true), _iterator() {}

    virtual ~Record() {}

    char *_key;
    char *_value;
    int _lifetime;
    bool _useIterator;
    MapIterator _iterator;
  };

  struct StringComparator : public Comparator {
    int compare(const void *first, const void *second) const {
      return strcmp((const char *)first, (const char *)second);
    }
  };

  struct STLStringComparator {
    bool operator()(const char *first, const char *second) const {
      return strcmp(first, second) < 0;
    }
  };

  char *generateKey();
  char *generateValue(int i, int j);
  int generateLifetime();
  void validateTree(ESTF::ResultCollector *collector);

  static const int _Iterations;
  static const int _Records;
  static NullLock _Lock;
  static StringComparator _Comparator;

  Record *_records;
  ESTF::Rand _rand;
  Map _map;
  std::multimap<const char *, char *, STLStringComparator> _stlMultiMap;
  std::map<const char *, char *, STLStringComparator> _stlMap;
};

ESTF_OBJECT_PTR(MapTest, ESTF::Component)

MapTest::StringComparator MapTest::_Comparator;
const int MapTest::_Iterations = 1000;
const int MapTest::_Records = 1000;

Mutex StlLock;

static const bool Debug = false;

MapTest::MapTest()
    : _records(0), _rand(), _map(_Comparator), _stlMultiMap(), _stlMap() {}

MapTest::~MapTest() {}

bool MapTest::run(ESTF::ResultCollector *collector) {
  Error error;
  bool stlResult = false;
  MapIterator iterator;

  _records = new Record[_Records];

  if (!_records) {
    ESTF_ERROR(collector, "Couldn't allocate memory");
    return false;
  }

  for (int k = 0; k < 3; ++k) {
    for (int i = 0; i < _Iterations; ++i) {
      for (int j = 0; j < _Records; ++j) {
        if (!_records[j]._key && 1 == _rand.generateRandom(1, 200)) {
          //
          // Create and insert a new node.
          //

          _records[j]._key = generateKey();
          _records[j]._value = generateValue(i, j);
          _records[j]._lifetime = i + generateLifetime();
          _records[j]._useIterator = (1 == _rand.generateRandom(1, 2));

          stlResult = _stlMap
                          .insert(std::pair<const char *, char *>(
                              (const char *)_records[j]._key,
                              (char *)_records[j]._value))
                          .second;

          if (_records[j]._useIterator) {
            error = _map.insert(_records[j]._key, _records[j]._value,
                                &_records[j]._iterator);

            if (ESB_SUCCESS == error) {
              ESTF_ASSERT(collector, !_records[j]._iterator.isNull());
            } else {
              ESTF_ASSERT(collector, _records[j]._iterator.isNull());
            }
          } else {
            error = _map.insert(_records[j]._key, _records[j]._value);
          }

          //
          //  Map inserts are allowed to fail if the STL insert
          //  failed.
          //
          ESTF_ASSERT(collector, stlResult == (ESB_SUCCESS == error));

          if (ESB_SUCCESS != error) {
            if (Debug) {
              std::cerr << "Failed to insert: " << (char *)_records[j]._key
                        << " at time " << i << std::endl;
            }

            delete[](char *) _records[j]._key;
            _records[j]._key = 0;
            delete[](char *) _records[j]._value;
            _records[j]._value = 0;
          } else {
            if (Debug) {
              std::cerr << "Inserted: " << (char *)_records[j]._key
                        << " (Map size: " << _map.size()
                        << " stl size: " << _stlMap.size() << ") at time " << i
                        << std::endl;
            }
          }

          validateTree(collector);
        }

        if (_records[j]._key && i == _records[j]._lifetime) {
          //
          //  Erase and delete an existing node.
          //

          if (_records[j]._useIterator) {
            error = _map.erase(&_records[j]._iterator);

            ESTF_ASSERT(collector, ESB_SUCCESS == error);

            _stlMap.erase((const char *)_records[j]._key);
          } else {
            error = _map.remove(_records[j]._key);

            ESTF_ASSERT(collector, ESB_SUCCESS == error);

            void *dummy = _map.find(_records[j]._key);

            ESTF_ASSERT(collector, 0 == dummy);

            _stlMap.erase((const char *)_records[j]._key);
          }

          if (Debug) {
            std::cerr << "Deleted: " << (char *)_records[j]._key
                      << " (Map size: " << _map.size()
                      << " stl size: " << _stlMap.size() << ") at time " << i
                      << std::endl;
          }

          delete[](char *) _records[j]._key;
          _records[j]._key = 0;
          delete[](char *) _records[j]._value;
          _records[j]._value = 0;

          validateTree(collector);
        }
      }
    }

    //
    //  Cleanup
    //

    MapIterator temp;
    char *key = 0;
    char *value = 0;

    for (iterator = _map.minimumIterator(); !iterator.isNull();
         iterator = temp) {
      key = (char *)iterator.key();
      value = (char *)iterator.value();

      _stlMap.erase(key);

      temp = iterator.next();

      error = _map.erase(&iterator);

      ESTF_ASSERT(collector, ESB_SUCCESS == error);

      if (Debug) {
        std::cerr << "Deleted: " << key << " (Map size: " << _map.size()
                  << " stl size: " << _stlMap.size() << ") at cleanup stage"
                  << std::endl;
      }

      delete[](char *) key;
      key = 0;
      delete[](char *) value;
      value = 0;

      validateTree(collector);
    }
  }

  delete[] _records;
  _records = 0;

  return true;
}

void MapTest::validateTree(ESTF::ResultCollector *collector) {
  MapIterator iterator;
  void *value = NULL;

  //
  //  Run through the stl map or multimap and verify that we can find
  //  every record in the map in the tree.
  //

  std::map<const char *, char *, STLStringComparator>::iterator it;

  for (it = _stlMap.begin(); it != _stlMap.end(); ++it) {
    value = _map.find(it->first);

    if (Debug && !value) {
      std::cerr << "Couldn't find: " << (char *)it->first << std::endl;
    }

    ESTF_ASSERT(collector, value);

    ESTF_ASSERT(collector, 0 == _Comparator.compare(value, it->second));
  }

  //
  //  Make sure that the tree is balanced.
  //

  ESTF_ASSERT(collector, true == _map.isBalanced());

  //
  //  Make sure our sizes are right.
  //

  ESTF_ASSERT(collector, _map.size() == _stlMap.size());

  if (2 > _map.size()) return;

  //
  //  Iterate through the map in forward order and make sure the keys
  //  are in ascending order.
  //

  MapIterator next;
  iterator = _map.minimumIterator();
  UInt32 i = 1;
  int comparison = 0;

  while (true) {
    next = iterator.next();

    if (next.isNull()) {
      break;
    }

    comparison = _Comparator.compare(next.key(), iterator.key());

    ESTF_ASSERT(collector, 0 <= comparison);

    iterator = next;
    ++i;
  }

  ESTF_ASSERT(collector, i == _map.size());

  //
  //  Iterate through the map in reverse order and make sure the keys
  //  are in descending order.
  //

  MapIterator prev;
  iterator = _map.maximumIterator();
  i = 1;
  comparison = 0;

  while (true) {
    prev = iterator.previous();

    if (prev.isNull()) {
      break;
    }

    comparison = _Comparator.compare(prev.key(), iterator.key());

    ESTF_ASSERT(collector, 0 >= comparison);

    iterator = prev;
    ++i;
  }

  ESTF_ASSERT(collector, i == _map.size());
}

bool MapTest::setup() { return true; }

bool MapTest::tearDown() { return true; }

ESTF::ComponentPtr MapTest::clone() {
  ESTF::ComponentPtr component(new MapTest());
  return component;
}

char *MapTest::generateKey() {
  char *key = new char[4];

  if (!key) return 0;

  for (int i = 0; i < 3; ++i) {
    key[i] = _rand.generateRandom(65, 90);
  }

  key[3] = '\0';

  return key;
}

char *MapTest::generateValue(int i, int j) {
  char *value = new char[22];

  if (!value) return 0;

  sprintf(value, "%d@%d", i, j);

  return value;
}

int MapTest::generateLifetime() {
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
  ESB::MapTestPtr mapTest = new ESB::MapTest();
  ESTF::ConcurrencyDecoratorPtr mapDecorator =
      new ESTF::ConcurrencyDecorator(mapTest, 10);
  ESTF::CompositePtr testSuite = new ESTF::Composite();
  testSuite->add(mapDecorator);
  ESTF::RepetitionDecoratorPtr root =
      new ESTF::RepetitionDecorator(testSuite, 3);
  ESTF::ResultCollector collector;

  if (!root->setup()) {
    std::cerr << "Testing framework setup failed" << std::endl;
    return 1;
  }

  if (!root->run(&collector)) {
    std::cerr << "Testing framework run failed" << std::endl;
    return 1;
  }

  if (!root->tearDown()) {
    std::cerr << "Testing framework tear down failed" << std::endl;
    return 1;
  }

  std::cout << collector << std::endl;

  return collector.getStatus();
}
