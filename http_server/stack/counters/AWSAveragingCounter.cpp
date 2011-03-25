/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_AVERAGING_COUNTER_H
#include <AWSAveragingCounter.h>
#endif

AWSAveragingCounter::AWSAveragingCounter() :
    _value(0.0), _observations(0.0), _lock() {
}

AWSAveragingCounter::~AWSAveragingCounter() {
}

void AWSAveragingCounter::addValue(double value) {
    _lock.writeAcquire();

    _observations = _observations + 1.0;

    _value = (value * (1.0 / _observations)) + (_value * ((_observations - 1.0) / _observations));

    _lock.writeRelease();
}

