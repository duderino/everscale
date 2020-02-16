/** @file ESFDiscardAllocatorTest.h
 *  @brief ESFDiscardAllocatorTest is the unit test for ESFDiscardAllocator.
 *
 *  Copyright 2005 Yahoo! Inc.
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:14 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_DISCARD_ALLOCATOR_TEST_H
#define ESF_DISCARD_ALLOCATOR_TEST_H

#ifndef ESTF_RESULT_COLECTOR_H
#include <ESTFResultCollector.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_H
#include <ESTFComponent.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

#ifndef ESTF_RAND_H
#include <ESTFRand.h>
#endif

/** ESTFDiscardAllocatorTest is the unit test for ESFDiscardAllocator.
 *
 *  @ingroup foundation_test
 */
class ESFDiscardAllocatorTest : public ESTFComponent {
 public:
  /** Constructor.
   */
  ESFDiscardAllocatorTest();

  /** Destructor. */
  virtual ~ESFDiscardAllocatorTest();

  /** Run the component.
   *
   *  @param collector A result collector that will collect the results of
   *      this test run.
   *  @return true if the test run was successfully performed by the test
   *      framework.  Application errors discovered during a test run do not
   *      count, a false return means there was an error in the test suite
   *      itself that prevented it from completing one or more test cases.
   */
  bool run(ESTFResultCollector *collector);

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
  ESTFComponentPtr clone();

 private:
  int generateAllocSize();

  ESTFRand _rand;
  ESFDiscardAllocator _allocator;
};

DEFINE_ESTF_OBJECT_PTR(ESFDiscardAllocatorTest, ESTFComponent)

#endif /* ! ESF_DISCARD_ALLOCATOR_TEST_H */
