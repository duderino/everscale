/** @file ESFCommandThread.h
 *  @brief A thread that runs an ESFCommand
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_COMMAND_THREAD_H
#define ESF_COMMAND_THREAD_H

#ifndef ESF_THREAD_H
#include <ESFThread.h>
#endif

#ifndef ESF_COMMAND_H
#include <ESFCommand.h>
#endif

/** A thread that runs an ESFCommand
 *
 *  @ingroup thread
 */
class ESFCommandThread: public ESFThread {
public:
    /** Constructor
     *
     * @param command The command to run in a separate thread of control
     */
    ESFCommandThread(ESFCommand *command);

    /** Default destructor.
     */
    virtual ~ESFCommandThread();

    /** Placement new.
     *
     *  @param size The size of the object.
     *  @param allocator The source of the object's memory.
     *  @return The new object or NULL of the memory allocation failed.
     */
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

protected:

    virtual void run();

    /** Should be called from a subclass's run method on every iteration to
     *  determine whether it should return from its run method.
     *
     *  @return true if a stop has been requested, false otherwise.
     */
    bool isStopRequested();

private:

    //  Disabled
    ESFCommandThread(const ESFCommandThread &);
    ESFCommandThread &operator=(const ESFCommandThread &);

    ESFCommand *_command;
};

#endif /* ! ESF_COMMAND_THREAD_H */
