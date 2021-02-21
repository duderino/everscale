#ifndef ES_HTTP_CLIENT_SIMPLE_COUNTERS_H
#define ES_HTTP_CLIENT_SIMPLE_COUNTERS_H

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ESB_SIMPLE_PERFORMANCE_COUNTER_H
#include <ESBSimplePerformanceCounter.h>
#endif

namespace ES {

class HttpClientSimpleCounters : public HttpClientCounters {
 public:
  HttpClientSimpleCounters();

  virtual ~HttpClientSimpleCounters();

  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const;

  virtual ESB::PerformanceCounter *getSuccesses();

  virtual const ESB::PerformanceCounter *getSuccesses() const;

  virtual ESB::PerformanceCounter *getFailures();

  virtual const ESB::PerformanceCounter *getFailures() const;

 private:
  // Disabled
  HttpClientSimpleCounters(const HttpClientSimpleCounters &counters);
  void operator=(const HttpClientSimpleCounters &counters);

  ESB::SimplePerformanceCounter _successes;
  ESB::SimplePerformanceCounter _failures;
};

}  // namespace ES

#endif
