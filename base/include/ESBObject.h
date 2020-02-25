#ifndef ESB_OBJECT_H
#define ESB_OBJECT_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

namespace ESB {

/** An object
 *
 *  @ingroup collection
 */
class Object {
 public:
  Object(){};

  virtual ~Object(){};
};

}  // namespace ESB

#endif
