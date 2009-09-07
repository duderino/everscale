/** @file ESFCommand.h
 *  @brief The interface for commands run by threadpools, etc.
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

#ifndef ESF_COMMAND_H
#define ESF_COMMAND_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_EMBEDDED_LIST_ELEMENT_H
#include <ESFEmbeddedListElement.h>
#endif

#ifndef ESF_FLAG_H
#include <ESFFlag.h>
#endif

/** @defgroup thread Thread
 */

/** The interface for commands run by threadpools, etc.
 *
 *  @ingroup thread
 */
class ESFCommand: public ESFEmbeddedListElement {
public:

    /** Constructor
     *
     */
    ESFCommand();

    /** Destructor.
     */
    virtual ~ESFCommand();

    /** Get the name of the command.  This name can be used in logging messages, etc.
     *
     * @return The command's name
     */
    virtual const char *getName() const = 0;

    /** Run the command
     *
     * @param isRunning This object will return true as long as the controlling thread
     *  isRunning, false when the controlling thread wants to shutdown.
     * @return If true, caller should destroy the command with the CleanupHandler.
     */
    virtual bool run(ESFFlag *isRunning) = 0;

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
    ESFCommand(const ESFCommand &);
    ESFCommand &operator=(const ESFCommand &);
};

#endif /* ! ESF_COMMAND_H */
