#ifndef ESB_SYSTEM_CONFIG_H
#define ESB_SYSTEM_CONFIG_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** Get OS configuration
 *
 *  @ingroup util
 */
class SystemConfig {
 public:
  inline static SystemConfig &Instance() { return _Instance; }

  virtual ~SystemConfig();

  UInt32 socketSoftMax();

  UInt32 socketHardMax();

  Error setSocketSoftMax(UInt32 limit);

 private:
  // Singleton
  SystemConfig();

  static SystemConfig _Instance;

  ESB_DISABLE_AUTO_COPY(SystemConfig);
};

}  // namespace ESB

#endif
