#ifndef ESB_SMART_POINTER_DEBUGGER_H
#define ESB_SMART_POINTER_DEBUGGER_H

#ifdef USE_SMART_POINTER_DEBUGGER

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_MAP_H
#include <ESBMap.h>
#endif

#ifndef ESB_REFERENCE_COUNT_H
#include <ESBReferenceCount.h>
#endif

namespace ESB {

/** This class should be used for debugging purposes only as it will cause
 *  substantial performance degradation when used.  When the controlling
 *  USE_SMART_POINTER_DEBUGGER macro is defined, every ReferenceCount
 *  object that is created and assigned to an SmartPointer or SmartPointer
 *  subclass will be tracked by the SmartPointerDebugger.  At any point, the
 *  SmartPointerDebugger can be queried for the number of outstanding objects
 *
 *  @ingroup object
 */
class SmartPointerDebugger {
 public:
  /** Get an instance of the global SmartPointerDebugger
   *
   *  @return A SmartPointerDebugger reference
   */
  static SmartPointerDebugger *Instance();

  /** Initialize the global SmartPointerDebugger
   *
   *  @return true if successful, false otherwise.
   */
  static bool Initialize();

  /** Destroy the global SmartPointerDebugger
   *
   *  @return true if successful, false otherwise.
   */
  static bool Destroy();

  /** Destructor */
  virtual ~SmartPointerDebugger();

  /** Register an ReferenceCount instance with the debugger.  This method
   *  will be called when a ReferenceCount or ReferenceCount subclass is
   *  assigned to an SmartPointer or SmartPointer subclass.  This
   *  method is idempotent.  It can be called multiple times for the same
   *  ReferenceCount or ReferenceCount subclass without ill effect.
   *
   *  @param object The object to register.
   */
  void insert(ReferenceCount *object);

  /** Unregister an ReferenceCount instance from the debugger.  This method
   *  will be called whenever an ReferenceCount or ReferenceCount
   *  subclass's reference count is decremented to 0.
   *
   *  @param object The object to unregister.
   */
  void erase(ReferenceCount *object);

  /** Get the current number of registered objects.  That is, the current
   *  number of active object references.
   *
   *  @return The current number of registered objects.
   */
  int getSize();

 private:
  /** Constructor */
  SmartPointerDebugger();

  // Disabled
  SmartPointerDebugger(const SmartPointerDebugger &);
  // Disabled
  SmartPointerDebugger *operator=(const SmartPointerDebugger &);

  static SmartPointerDebugger _Instance;

  Map _references;
};

}  // namespace ESB

#endif /* defined USE_SMART_POINTER_DEBUGGER */
#endif /* ! defined ESB_SMART_POINTER_DEBUGGER_H */
