#ifndef ESB_SMART_POINTER_DEBUGGER_H
#include <ESBSmartPointerDebugger.h>
#endif

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_SYSTEM_ALLOCATOR_H
#include <ESBSystemAllocator.h>
#endif

namespace ESB {

SmartPointerDebugger SmartPointerDebugger::_Instance;

/** @todo move this to Map? */

class AddressComparator : public Comparator {
 public:
  virtual int compare(const void *first, const void *second) const;
};

int AddressComparator::compare(const void *first, const void *second) const {
  if (first < second) {
    return -1;
  } else if (first > second) {
    return 1;
  }

  return 0;
}

static AddressComparator AddressComparator;
static Mutex SmartPointerDebuggerMutex;

SmartPointerDebugger *SmartPointerDebugger::Instance() { return &_Instance; }

bool SmartPointerDebugger::Initialize() { return true; }

bool SmartPointerDebugger::Destroy() { return true; }

SmartPointerDebugger::~SmartPointerDebugger() {}

void SmartPointerDebugger::insert(ReferenceCount *object) {
  _references.writeAcquire();

  _references.insert(object, object);

  _references.writeRelease();
}

void SmartPointerDebugger::erase(ReferenceCount *object) {
  _references.writeAcquire();

  _references.erase(object);

  _references.writeRelease();
}

int SmartPointerDebugger::getSize() {
  int size;

  _references.readAcquire();

  size = _references.getSize();

  _references.readRelease();

  return size;
}

SmartPointerDebugger::SmartPointerDebugger()
    : _references(true, &AddressComparator, SystemAllocator::GetInstance(),
                  &SmartPointerDebuggerMutex) {}

}  // namespace ESB
