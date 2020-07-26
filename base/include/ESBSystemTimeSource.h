#ifndef ESB_SYSTEM_TIME_SOURCE_H
#define ESB_SYSTEM_TIME_SOURCE_H

#ifndef ESB_TIME_SOURCE_H
#include <ESBTimeSource.h>
#endif

namespace ESB {

/** A TimeSource impl that gets the current time from the operating system.
 *
 *  @ingroup util
 */
class SystemTimeSource : public TimeSource {
 public:
  /** Constructor */
  static inline SystemTimeSource &Instance() { return _Instance; }

  /** Destructor. */
  virtual ~SystemTimeSource();

  virtual Date now();

 private:
  //  Disabled
  SystemTimeSource();
  SystemTimeSource(const TimeSource &);
  SystemTimeSource &operator=(const TimeSource &);

  static SystemTimeSource _Instance;
};

}  // namespace ESB

#endif
