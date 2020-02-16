/** @file ESFSmartPointerDebugger.cpp
 *  @brief A global map that can track the creation and destruction of all
 *      reference-counted objects used by smart pointers in the system
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_SMART_POINTER_DEBUGGER_H
#include <ESFSmartPointerDebugger.h>
#endif

#ifdef USE_SMART_POINTER_DEBUGGER

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

ESFSmartPointerDebugger ESFSmartPointerDebugger::_Instance;

/** @todo move this to ESFMap? */

class ESFAddressComparator : public ESFComparator {
 public:
  virtual int compare(const void *first, const void *second) const;
};

int ESFAddressComparator::compare(const void *first, const void *second) const {
  if (first < second) {
    return -1;
  } else if (first > second) {
    return 1;
  }

  return 0;
}

static ESFAddressComparator AddressComparator;
static ESFMutex SmartPointerDebuggerMutex;

ESFSmartPointerDebugger *ESFSmartPointerDebugger::Instance() {
  return &_Instance;
}

bool ESFSmartPointerDebugger::Initialize() { return true; }

bool ESFSmartPointerDebugger::Destroy() { return true; }

ESFSmartPointerDebugger::~ESFSmartPointerDebugger() {}

void ESFSmartPointerDebugger::insert(ESFReferenceCount *object) {
  _references.writeAcquire();

  _references.insert(object, object);

  _references.writeRelease();
}

void ESFSmartPointerDebugger::erase(ESFReferenceCount *object) {
  _references.writeAcquire();

  _references.erase(object);

  _references.writeRelease();
}

int ESFSmartPointerDebugger::getSize() {
  int size;

  _references.readAcquire();

  size = _references.getSize();

  _references.readRelease();

  return size;
}

ESFSmartPointerDebugger::ESFSmartPointerDebugger()
    : _references(true, &AddressComparator, ESFSystemAllocator::GetInstance(),
                  &SmartPointerDebuggerMutex) {}

#endif /* defined USE_SMART_POINTER_DEBUGGER */
