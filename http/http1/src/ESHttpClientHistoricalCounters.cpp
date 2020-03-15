#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
#endif

namespace ES {

HttpClientHistoricalCounters::HttpClientHistoricalCounters(
    ESB::UInt16 maxWindows, ESB::UInt16 windowSizeSec,
    ESB::Allocator *allocator)
    : _successes("CLIENT TRANS SUCCESS", maxWindows, windowSizeSec, allocator),
      _failures("CLIENT TRANS FAILURE", maxWindows, windowSizeSec, allocator) {}

HttpClientHistoricalCounters::~HttpClientHistoricalCounters() {}

void HttpClientHistoricalCounters::printSummary(FILE *file) const {
  _successes.printSummary(file);
  _failures.printSummary(file);
}

ESB::PerformanceCounter *HttpClientHistoricalCounters::getSuccesses() {
  return &_successes;
}

const ESB::PerformanceCounter *HttpClientHistoricalCounters::getSuccesses()
    const {
  return &_successes;
}

ESB::PerformanceCounter *HttpClientHistoricalCounters::getFailures() {
  return &_failures;
}

const ESB::PerformanceCounter *HttpClientHistoricalCounters::getFailures()
    const {
  return &_failures;
}

}  // namespace ES
