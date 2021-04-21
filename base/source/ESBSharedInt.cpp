#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ESB {

SharedInt::SharedInt() : _counter(0) {}

SharedInt::SharedInt(UInt32 value) : _counter(value) {}

SharedInt::SharedInt(SharedInt &counter) { set(counter.get()); }

SharedInt::~SharedInt() {}

SharedInt &SharedInt::operator=(SharedInt &counter) {
  set(counter.get());
  return *this;
}

}  // namespace ESB
