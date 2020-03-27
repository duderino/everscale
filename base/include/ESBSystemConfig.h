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

  inline UInt32 pageSize() { return _pageSize; }

  inline UInt32 cacheLineSize() { return _cacheLineSize; }

  UInt32 socketSoftMax();

  UInt32 socketHardMax();

  Error setSocketSoftMax(UInt32 limit);

 private:
  // Singleton
  SystemConfig();
  //  Disabled
  SystemConfig(const SystemConfig &);
  SystemConfig &operator=(const SystemConfig &);

  UInt32 _pageSize;
  UInt32 _cacheLineSize;
  static SystemConfig _Instance;
};

}  // namespace ESB

#endif
