#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#include <ESHttpClientHistoricalCounters.h>
#endif

namespace ES {

HttpClientHistoricalCounters::HttpClientHistoricalCounters(
    time_t windowSizeSec, ESB::Allocator *allocator, ESB::Logger *logger)
    : _successes("CLIENT TRANS SUCCESS", windowSizeSec, allocator, logger),
      _failures("CLIENT TRANS FAILURE", windowSizeSec, allocator, logger) {}

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
