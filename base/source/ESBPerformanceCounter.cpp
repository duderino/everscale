#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

namespace ESB {

PerformanceCounter::PerformanceCounter() : EmbeddedListElement() {}

PerformanceCounter::~PerformanceCounter() {}

CleanupHandler *PerformanceCounter::cleanupHandler() { return 0; }

}  // namespace ESB
