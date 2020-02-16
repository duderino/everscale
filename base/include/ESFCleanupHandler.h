#ifndef ESF_CLEANUP_HANDLER_H
#define ESF_CLEANUP_HANDLER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_OBJECT_H
#include <ESFObject.h>
#endif

/** An object that can destroy another object
 *
 *  @ingroup collection
 */
class ESFCleanupHandler {
 public:
  /** Constructor
   */
  ESFCleanupHandler(){};

  /** Destructor
   */
  virtual ~ESFCleanupHandler(){};

  /** Destroy an object
   *
   * @param object The object to destroy
   */
  virtual void destroy(ESFObject *object) = 0;

 private:
  // Disabled
  ESFCleanupHandler(const ESFCleanupHandler &);
  void operator=(const ESFCleanupHandler &);
};

#endif /* ! ESF_CLEANUP_HANDLER_H */
