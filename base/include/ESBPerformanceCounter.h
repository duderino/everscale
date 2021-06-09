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

  virtual void record(const Date &start, const Date &stop) = 0;

  virtual UInt32 queries() const = 0;

  virtual void log(Logger &logger, Logger::Severity severity = Logger::Severity::Debug) const = 0;

  virtual CleanupHandler *cleanupHandler();

  ESB_DEFAULT_FUNCS(PerformanceCounter);
};

}  // namespace ESB

#endif
