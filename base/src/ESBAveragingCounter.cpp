#ifndef ESB_AVERAGING_COUNTER_H
#include <ESBAveragingCounter.h>
#endif

namespace ESB {

AveragingCounter::AveragingCounter()
    : _value(0.0), _observations(0.0), _lock() {}

AveragingCounter::~AveragingCounter() {}

void AveragingCounter::addValue(double value) {
  _lock.writeAcquire();

  _observations = _observations + 1.0;

  _value = (value * (1.0 / _observations)) +
           (_value * ((_observations - 1.0) / _observations));

  _lock.writeRelease();
}

}
