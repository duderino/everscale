/** @file ESFSocketMultiplexer.cpp
 *  @brief A thread that delegates i/o readiness events to multiple ESFMultiplexedSockets
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

#ifndef ESF_SOCKET_MULTIPLEXER_H
#include <ESFSocketMultiplexer.h>
#endif

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

ESFSocketMultiplexer::ESFSocketMultiplexer(const char *name, ESFLogger *logger) :
    _name(name ? name : "SocketMultiplexer"), _logger(logger ? logger : ESFNullLogger::GetInstance()) {
}

ESFSocketMultiplexer::~ESFSocketMultiplexer() {
}

