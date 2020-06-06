#ifndef ESB_AVERAGING_COUNTER_H
#include <ESBAveragingCounter.h>
#endif

#ifndef ESB_READ_SCOPE_LOCK_H
#include <ESBReadScopeLock.h>
#endif

#ifndef EBS_WRITE_SCOPE_LOCK_H
#include <ESBWriteScopeLock.h>
#endif

namespace ESB {

AveragingCounter::AveragingCounter() : _mean(0.0), _avgDistToMeanSq(0.0), _min(0.0), _max(0.0), _n(0.0) {}

AveragingCounter::~AveragingCounter() {}

void AveragingCounter::add(const double value) {
  // Welford's online algorithm
  const double n = ++_n;
  const double delta = value - _mean;
  _mean = (n - 1) * _mean / n + value / n;
  const double delta2 = value - _mean;
  _avgDistToMeanSq += delta * delta2;

  if (1 == _n) {
    _min = value;
    _max = value;
    return;
  }

  if (value < _min) {
    _min = value;
  } else if (_max < value) {
    _max = value;
  }
}
void AveragingCounter::log(Logger &logger, Logger::Severity severity, const char *description) const {
  ESB_LOG(logger, severity, "%s: N=%u, MEAN=%.2lf, VAR=%.2f, MIN=%.2lf, MAX=%.2lf", description, n(), mean(),
          variance(), min(), max());
}

}  // namespace ESB
