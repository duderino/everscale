/** @file ESFNullLock.cpp
 *  @brief An implementation of the ESFLockable interface that provides no
 *      synchronization whatsoever
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

#ifndef ESF_NULL_LOCK_H
#include <ESFNullLock.h>
#endif

ESFNullLock ESFNullLock::_Instance;

ESFNullLock *ESFNullLock::Instance() { return &_Instance; }

ESFNullLock::ESFNullLock() {}

ESFNullLock::~ESFNullLock() {}

ESFError ESFNullLock::writeAcquire() { return ESF_SUCCESS; }

ESFError ESFNullLock::readAcquire() { return ESF_SUCCESS; }

ESFError ESFNullLock::writeAttempt() { return ESF_SUCCESS; }

ESFError ESFNullLock::readAttempt() { return ESF_SUCCESS; }

ESFError ESFNullLock::writeRelease() { return ESF_SUCCESS; }

ESFError ESFNullLock::readRelease() { return ESF_SUCCESS; }
