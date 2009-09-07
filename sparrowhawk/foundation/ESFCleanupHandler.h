/** @file ESFCleanupHandler.h
 *  @brief An object that can destroy another object
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

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
    ESFCleanupHandler() {
    }
    ;

    /** Destructor
     */
    virtual ~ESFCleanupHandler() {
    }
    ;

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
