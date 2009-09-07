/** @file ESFSocketMultiplexerFactory.h
 *  @brief An abstract factory that creates socket multiplexers
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

#ifndef ESF_SOCKET_MULTIPLEXER_FACTORY_H
#define ESF_SOCKET_MULTIPLEXER_FACTORY_H

#ifndef ESF_SOCKET_MULTIPLEXER_H
#include <ESFSocketMultiplexer.h>
#endif

/** An abstract factory that creates socket multiplexers
 *
 * @ingroup network
 */
class ESFSocketMultiplexerFactory {
public:

    /** Constructor.
     */
    ESFSocketMultiplexerFactory();

    /** Destructor.
     */
    virtual ~ESFSocketMultiplexerFactory();

    /** Create a new socket multiplexer
     *
     * @param maxSockets The maxium number of sockets the multiplexer should handle.
     * @return a new socket multiplexer or NULL if it could not be created
     */
    virtual ESFSocketMultiplexer *create(int maxSockets) = 0;

    /** Destroy a socket multiplexer
     *
     * @param multiplexer The socket multiplexer to destroy.
     */
    virtual void destroy(ESFSocketMultiplexer *multiplexer) = 0;

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

    //  Disabled
    ESFSocketMultiplexerFactory(const ESFSocketMultiplexerFactory &);
    ESFSocketMultiplexerFactory &operator=(const ESFSocketMultiplexerFactory &);
};

#endif /* ! ESF_SOCKET_MULTIPLEXER_FACTORY_H */
