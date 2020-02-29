#ifndef ESB_PROCESS_LIMITS_H
#define ESB_PROCESS_LIMITS_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

namespace ESB {

/** Utility functions for accessing process-wide resource limits.
 *
 * @ingroup thread
 */
class ProcessLimits {
 public:
  static UInt32 GetSocketSoftMax();

  static UInt32 GetSocketHardMax();

  static Error SetSocketSoftMax(UInt32 limit);

 private:
  //  Disabled
  ProcessLimits();
  ~ProcessLimits();
  ProcessLimits(const ProcessLimits &);
  ProcessLimits &operator=(const ProcessLimits &);
};

}

#endif
