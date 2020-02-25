#ifndef ESB_PERFORMANCE_COUNTER_H
#define ESB_PERFORMANCE_COUNTER_H

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifndef HAVE_STRUCT_TIMEVAL
#error "struct timeval or equivalent is required"
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifndef HAVE_FILE_T
#error "FILE or equivalent is required"
#endif

namespace ESB {

class PerformanceCounter : public EmbeddedListElement {
 public:
  PerformanceCounter();

  virtual ~PerformanceCounter();

  static void GetTime(struct timeval *now);

  virtual void addObservation(const struct timeval *start) = 0;

  virtual void addObservation(const struct timeval *start,
                              const struct timeval *stop) = 0;

  virtual void printSummary(FILE *file) const = 0;

  virtual CleanupHandler *getCleanupHandler();

 private:
  // Disabled
  PerformanceCounter(const PerformanceCounter &counter);
  PerformanceCounter *operator=(const PerformanceCounter &counter);
};

}  // namespace ESB

#endif
