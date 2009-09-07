/** @file ESFEpollMultiplexerFactory.cpp
 *  @brief A factory that creates epoll socket multiplexers
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

#ifndef ESF_EPOLL_MULTIPLEXER_FACTORY_H
#include <ESFEpollMultiplexerFactory.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

ESFEpollMultiplexerFactory::ESFEpollMultiplexerFactory(const char *name, ESFLogger *logger, ESFAllocator *allocator) :
    _name(name), _logger(logger ? logger : ESFNullLogger::GetInstance()), _allocator(allocator ? allocator
            : ESFSystemAllocator::GetInstance()) {
}

ESFEpollMultiplexerFactory::~ESFEpollMultiplexerFactory() {
}

ESFSocketMultiplexer *ESFEpollMultiplexerFactory::create(int maxSockets) {
    return new (_allocator) ESFEpollMultiplexer(_name, maxSockets, _logger, _allocator);
}

void ESFEpollMultiplexerFactory::destroy(ESFSocketMultiplexer *multiplexer) {
    if (!multiplexer) {
        return;
    }

    multiplexer->~ESFSocketMultiplexer();
    _allocator->deallocate(multiplexer);
}

