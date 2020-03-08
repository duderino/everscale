#ifndef ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H
#define ES_HTTP_CLIENT_HISTORICAL_COUNTERS_H

#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

#ifndef ESB_HISTORICAL_PERFORMANCE_COUNTER_H
#include <ESBHistoricalPerformanceCounter.h>
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
  HttpClientHistoricalCounters(time_t windowSizeSec, ESB::Allocator *allocator);

  virtual ~HttpClientHistoricalCounters();

  virtual void printSummary(FILE *file) const;

  virtual ESB::PerformanceCounter *getSuccesses();

  virtual const ESB::PerformanceCounter *getSuccesses() const;

  virtual ESB::PerformanceCounter *getFailures();

  virtual const ESB::PerformanceCounter *getFailures() const;

 private:
  // Disabled
  HttpClientHistoricalCounters(const HttpClientHistoricalCounters &counters);
  void operator=(const HttpClientHistoricalCounters &counters);

  ESB::HistoricalPerformanceCounter _successes;
  ESB::HistoricalPerformanceCounter _failures;
};

}  // namespace ES

#endif
