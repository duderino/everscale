#ifndef ESB_SYSTEM_CONFIG_H
#define ESB_SYSTEM_CONFIG_H

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
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
  //  Disabled
  SystemConfig(const SystemConfig &);
  SystemConfig &operator=(const SystemConfig &);

  static SystemConfig _Instance;
};

}  // namespace ESB

#endif
