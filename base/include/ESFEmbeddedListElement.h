/** @file ESFEmbeddedListElement.h
 *  @brief An element that can be stored in a ESFEmbeddedList
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#define ESF_EMBEDDED_LIST_ELEMENT_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_OBJECT_H
#include <ESFObject.h>
#endif

#ifndef ESF_CLEANUP_HANDLER_H
#include <ESFCleanupHandler.h>
#endif

/** An element that can be stored in a ESFEmbeddedList
 *
 *  @ingroup collection
 */
class ESFEmbeddedListElement : public ESFObject {
 public:
  /** Constructor.
   */
  ESFEmbeddedListElement();

  /** Destructor.
   */
  virtual ~ESFEmbeddedListElement();

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESFCleanupHandler *getCleanupHandler() = 0;

  inline ESFEmbeddedListElement *getNext() { return _next; }

  inline const ESFEmbeddedListElement *getNext() const { return _next; }

  inline void setNext(ESFEmbeddedListElement *next) { _next = next; }

  inline ESFEmbeddedListElement *getPrevious() { return _previous; }

  inline const ESFEmbeddedListElement *getPrevious() const { return _previous; }

  inline void setPrevious(ESFEmbeddedListElement *previous) {
    _previous = previous;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  ESFEmbeddedListElement(const ESFEmbeddedListElement &);
  ESFEmbeddedListElement &operator=(const ESFEmbeddedListElement &);

  ESFEmbeddedListElement *_next;
  ESFEmbeddedListElement *_previous;
};

#endif /* ! ESF_EMBEDDED_LIST_ELEMENT_H */
