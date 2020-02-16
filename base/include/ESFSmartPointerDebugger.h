/** @file ESFSmartPointerDebugger.h
 *  @brief A global map that can track the creation and destruction of all
 *      reference-counted objects used by smart pointers in the system
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 * $Author: blattj $
 * $Date: 2009/05/25 21:51:08 $
 * $Name:  $
 * $Revision: 1.3 $
 */

#ifndef ESF_SMART_POINTER_DEBUGGER_H
#define ESF_SMART_POINTER_DEBUGGER_H

#ifdef USE_SMART_POINTER_DEBUGGER

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_MAP_H
#include <ESFMap.h>
#endif

#ifndef ESF_REFERENCE_COUNT_H
#include <ESFReferenceCount.h>
#endif

/** This class should be used for debugging purposes only as it will cause
 *  substantial performance degradation when used.  When the controlling
 *  USE_SMART_POINTER_DEBUGGER macro is defined, every ESFReferenceCount
 *  object that is created and assigned to an ESFSmartPointer or ESFSmartPointer
 *  subclass will be tracked by the ESFSmartPointerDebugger.  At any point, the
 *  ESFSmartPointerDebugger can be queried for the number of outstanding objects
 *
 *  @ingroup object
 */
class ESFSmartPointerDebugger {
 public:
  /** Get an instance of the global ESFSmartPointerDebugger
   *
   *  @return A ESFSmartPointerDebugger reference
   */
  static ESFSmartPointerDebugger *Instance();

  /** Initialize the global ESFSmartPointerDebugger
   *
   *  @return true if successful, false otherwise.
   */
  static bool Initialize();

  /** Destroy the global ESFSmartPointerDebugger
   *
   *  @return true if successful, false otherwise.
   */
  static bool Destroy();

  /** Destructor */
  virtual ~ESFSmartPointerDebugger();

  /** Register an ESFReferenceCount instance with the debugger.  This method
   *  will be called when a ESFReferenceCount or ESFReferenceCount subclass is
   *  assigned to an ESFSmartPointer or ESFSmartPointer subclass.  This
   *  method is idempotent.  It can be called multiple times for the same
   *  ESFReferenceCount or ESFReferenceCount subclass without ill effect.
   *
   *  @param object The object to register.
   */
  void insert(ESFReferenceCount *object);

  /** Unregister an ESFReferenceCount instance from the debugger.  This method
   *  will be called whenever an ESFReferenceCount or ESFReferenceCount
   *  subclass's reference count is decremented to 0.
   *
   *  @param object The object to unregister.
   */
  void erase(ESFReferenceCount *object);

  /** Get the current number of registered objects.  That is, the current
   *  number of active object references.
   *
   *  @return The current number of registered objects.
   */
  int getSize();

 private:
  /** Constructor */
  ESFSmartPointerDebugger();

  // Disabled
  ESFSmartPointerDebugger(const ESFSmartPointerDebugger &);
  // Disabled
  ESFSmartPointerDebugger *operator=(const ESFSmartPointerDebugger &);

  static ESFSmartPointerDebugger _Instance;

  ESFMap _references;
};

#endif /* defined USE_SMART_POINTER_DEBUGGER */

#endif /* ! defined ESF_SMART_POINTER_DEBUGGER_H */
