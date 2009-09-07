/** @file ESFCommandThread.cpp
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
#include <ESFCommandThread.h>
#endif

ESFCommandThread::ESFCommandThread(ESFCommand *command) :
    _command(command) {
}

ESFCommandThread::~ESFCommandThread() {
}

void ESFCommandThread::run() {
    if (!_command) {
        return;
    }

    if (_command->run(&_isRunning)) {
        ESFCleanupHandler *cleanupHandler = _command->getCleanupHandler();

        if (cleanupHandler) {
            cleanupHandler->destroy(_command);
        }
    }
}

