#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#define ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ESB_TIME_SERIES_H
#include <ESBTimeSeries.h>
#endif

namespace ES {

class HttpClientHistoricalCounters : public HttpClientCounters {
 public:
  /**
   * Constructor.
   *
   * @param windowSizeSec The counter's latency and throughput will be saved
   *   for each window of this many seconds.
   * @param allocator The counter will grab memory from this allocator
   *   for each new window it creates.
   */
  HttpClientHistoricalCounters(ESB::UInt16 maxWindows,
                               ESB::UInt16 windowSizeSec,
                               ESB::Allocator *allocator);

  virtual ~HttpClientHistoricalCounters();

  virtual void log(ESB::Logger &logger, ESB::Logger::Severity severity) const;

  virtual ESB::PerformanceCounter *getSuccesses();

  virtual const ESB::PerformanceCounter *getSuccesses() const;

  virtual ESB::PerformanceCounter *getFailures();

  virtual const ESB::PerformanceCounter *getFailures() const;

 private:
  // Disabled
  HttpClientHistoricalCounters(const HttpClientHistoricalCounters &counters);
  void operator=(const HttpClientHistoricalCounters &counters);

  ESB::TimeSeries _successes;
  ESB::TimeSeries _failures;
};

}  // namespace ES

#endif
