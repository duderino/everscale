#ifndef EST_PERFORMANCE_COUNTER_H
#define EST_PERFORMANCE_COUNTER_H

#include <pthread.h>
#include <sys/time.h>

namespace EST {

class PerformanceCounter {
 public:
  PerformanceCounter(const char *name);

  virtual ~PerformanceCounter();

  static void GetTime(struct timeval *now);

  void addObservation(struct timeval *start);

  inline const char *getName() { return _name; }

  inline unsigned long getThroughput() { return _throughput; }

  inline double getAverageLatencyMsec() { return _avgLatencyMsec; }

  inline double getMinLatencyMsec() {
    return 0 > _minLatencyMsec ? 0 : _minLatencyMsec;
  }

  inline double getMaxLatencyMsec() { return _maxLatencyMsec; }

  void printSummary();

 private:
  // Disabled
  PerformanceCounter(const PerformanceCounter &counter);
  PerformanceCounter *operator=(const PerformanceCounter &counter);

  const char *_name;
  pthread_mutex_t _lock;
  double _avgLatencyMsec;
  double _minLatencyMsec;
  double _maxLatencyMsec;
  unsigned long _throughput;
};

}  // namespace EST

#endif
