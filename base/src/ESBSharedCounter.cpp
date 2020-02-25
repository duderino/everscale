#ifndef ESB_SHARED_COUNTER_H
#include <ESBSharedCounter.h>
#endif

namespace ESB {

SharedCounter::SharedCounter() : _counter(0) {}

SharedCounter::SharedCounter(SharedCounter &counter) { set(counter.get()); }

SharedCounter::~SharedCounter() {}

SharedCounter &SharedCounter::operator=(SharedCounter &counter) {
  set(counter.get());

  return *this;
}

}  // namespace ESB
