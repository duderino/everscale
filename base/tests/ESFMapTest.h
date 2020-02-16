/**	@file ESFMapTest.h
 *	@brief ESFMapTest is the unit test for ESFMap.
 *
 *  Copyright 2005 Joshua Blatt
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:14 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_MAP_TEST_H
#define ESF_MAP_TEST_H

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESF_MAP_H
#include <ESFMap.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

#ifndef ESF_NULL_LOCK_H
#include <ESFNullLock.h>
#endif

#ifndef ESF_COMPARATOR_H
#include <ESFComparator.h>
#endif

#include <map>

/**	ESTFMapTest is the unit test for ESFMap.
 *
 *  @ingroup foundation_test
 */
class ESFMapTest : public ESTFComponent {
 public:
  /**	Constructor.
   *
   *  @param isUnique If true, a map that enforces uniqueness will be tested,
   *      if false, a map that allows multiple elements with the same key
   *      will be tested.
   */
  ESFMapTest(bool isUnique);

  /** Destructor. */
  virtual ~ESFMapTest();

  /** Run the component.
   *
   *	@param collector A result collector that will collect the results of
   *		this test run.
   *	@return true if the test run was successfully performed by the test
   *		framework.  Application errors discovered during a test run do
   *not count, a false return means there was an error in the test suite itself
   *that prevented it from completing one or more test cases.
   */
  bool run(ESTFResultCollector *collector);

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
  ESTFComponentPtr clone();

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
    ESFMapIterator _iterator;
  };

  struct StringComparator : public ESFComparator {
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
  void validateTree(ESTFResultCollector *collector);

  static const int _Iterations;
  static const int _Records;
  static ESFNullLock _Lock;
  static StringComparator _Comparator;

  bool _isUnique;
  Record *_records;
  ESTFRand _rand;
  ESFMap _map;
  std::multimap<const char *, char *, STLStringComparator> _stlMultiMap;
  std::map<const char *, char *, STLStringComparator> _stlMap;
};

DEFINE_ESTF_OBJECT_PTR(ESFMapTest, ESTFComponent)

#endif /* ! ESF_MAP_TEST_H */
