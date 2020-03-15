#ifndef ESB_PERFORMANCE_COUNTER_H
#define ESB_PERFORMANCE_COUNTER_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_DATE_H
#include <ESBDate.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ESB {

class PerformanceCounter : public EmbeddedListElement {
 public:
  PerformanceCounter();

  virtual ~PerformanceCounter();

  virtual void addObservation(const Date &start, const Date &stop) = 0;

  virtual void log(Logger &logger, Logger::Severity severity =
                                       Logger::Severity::Debug) const = 0;

  virtual CleanupHandler *getCleanupHandler();

 private:
  // Disabled
  PerformanceCounter(const PerformanceCounter &counter);
  PerformanceCounter *operator=(const PerformanceCounter &counter);
};

}  // namespace ESB

#endif
