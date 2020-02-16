/** @file ESFNullLogger.cpp
 *  @brief A no-op implementation of the ESFLogger interface.
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

#ifndef ESF_NULL_LOGGER_H
#include <ESFNullLogger.h>
#endif

ESFNullLogger ESFNullLogger::_Instance;

ESFNullLogger::ESFNullLogger() {
}

ESFNullLogger::~ESFNullLogger() {
}

bool ESFNullLogger::isLoggable(Severity severity) {
    return false;
}

void ESFNullLogger::setSeverity(Severity severity) {
}

ESFError ESFNullLogger::log(Severity severity, const char *file, int line, const char *format, ...) {
    return ESF_SUCCESS;
}

