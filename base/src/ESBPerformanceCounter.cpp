#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

namespace ESB {

PerformanceCounter::PerformanceCounter() : EmbeddedListElement() {}

PerformanceCounter::~PerformanceCounter() {}

void PerformanceCounter::GetTime(struct timeval *now) {
  gettimeofday(now, 0);
}

CleanupHandler *PerformanceCounter::getCleanupHandler() { return 0; }

}
