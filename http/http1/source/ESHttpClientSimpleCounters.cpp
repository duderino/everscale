#ifndef ES_HTTP_CLIENT_SIMPLE_COUNTERS_H
#include <ESHttpClientSimpleCounters.h>
#endif

namespace ES {

HttpClientSimpleCounters::HttpClientSimpleCounters()
    : _successes("CLIENT TRANS SUCCESS"), _failures("CLIENT TRANS FAILURE") {}

HttpClientSimpleCounters::~HttpClientSimpleCounters() {}

void HttpClientSimpleCounters::log(ESB::Logger &logger, ESB::Logger::Severity severity) const {
  _successes.log(logger, severity);
  _failures.log(logger, severity);
}

ESB::PerformanceCounter *HttpClientSimpleCounters::getSuccesses() { return &_successes; }

const ESB::PerformanceCounter *HttpClientSimpleCounters::getSuccesses() const { return &_successes; }

ESB::PerformanceCounter *HttpClientSimpleCounters::getFailures() { return &_failures; }

const ESB::PerformanceCounter *HttpClientSimpleCounters::getFailures() const { return &_failures; }

}  // namespace ES
