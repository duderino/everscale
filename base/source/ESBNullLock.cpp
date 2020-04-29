#ifndef ESB_NULL_LOCK_H
#include <ESBNullLock.h>
#endif

namespace ESB {

NullLock NullLock::_Instance;

NullLock &NullLock::Instance() { return _Instance; }

NullLock::NullLock() {}

NullLock::~NullLock() {}

Error NullLock::writeAcquire() { return ESB_SUCCESS; }

Error NullLock::readAcquire() { return ESB_SUCCESS; }

Error NullLock::writeAttempt() { return ESB_SUCCESS; }

Error NullLock::readAttempt() { return ESB_SUCCESS; }

Error NullLock::writeRelease() { return ESB_SUCCESS; }

Error NullLock::readRelease() { return ESB_SUCCESS; }

}  // namespace ESB
