#ifndef ESB_CLEANUP_HANDLER_H
#define ESB_CLEANUP_HANDLER_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

#ifndef ESB_ERROR_H
#include <ESBError.h>
#endif

#ifndef ESB_OBJECT_H
#include <ESBObject.h>
#endif

namespace ESB {

/** An object that can destroy another object
 *
 *  @ingroup collection
 */
class CleanupHandler {
 public:
  /** Constructor
   */
  CleanupHandler(){};

  /** Destructor
   */
  virtual ~CleanupHandler(){};

  /** Destroy an object
   *
   * @param object The object to destroy
   */
  virtual void destroy(Object *object) = 0;

  ESB_DISABLE_AUTO_COPY(CleanupHandler);
};

}  // namespace ESB

#endif
