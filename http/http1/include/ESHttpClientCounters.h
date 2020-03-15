#ifndef ES_HTTP_CLIENT_COUNTERS_H
#define ES_HTTP_CLIENT_COUNTERS_H

#ifndef ESB_PERFORMANCE_COUNTER_H
#include <ESBPerformanceCounter.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ES {

class HttpClientCounters {
 public:
  HttpClientCounters();

  virtual ~HttpClientCounters();

  virtual void log(ESB::Logger &logger,
                   ESB::Logger::Severity severity) const = 0;

  virtual ESB::PerformanceCounter *getSuccesses() = 0;

  virtual const ESB::PerformanceCounter *getSuccesses() const = 0;

  virtual ESB::PerformanceCounter *getFailures() = 0;

  virtual const ESB::PerformanceCounter *getFailures() const = 0;

 private:
  // Disabled
  HttpClientCounters(const HttpClientCounters &counters);
  void operator=(const HttpClientCounters &counters);
};

}  // namespace ES

#endif
