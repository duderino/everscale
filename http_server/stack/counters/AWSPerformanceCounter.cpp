/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#include <stdio.h>

AWSPerformanceCounter::AWSPerformanceCounter(const char *name) :
    _name(name),
    _avgLatencyMsec(0.0),
    _minLatencyMsec(-1.0),
    _maxLatencyMsec(0.0),
    _throughput(0UL),
    _lock()
{
}


AWSPerformanceCounter::~AWSPerformanceCounter()
{
}

void AWSPerformanceCounter::GetTime(struct timeval *now)
{
    gettimeofday(now, 0);
}

void AWSPerformanceCounter::addObservation(const struct timeval *start)
{
    struct timeval now;

    gettimeofday(&now, 0);

    double latencyMsec = (now.tv_sec - start->tv_sec) * 1000.0;
    latencyMsec += now.tv_usec / 1000.0;
    latencyMsec -= start->tv_usec / 1000.0;

    _lock.writeAcquire();

    ++_throughput;

    double throughput = _throughput;

    _avgLatencyMsec = (latencyMsec * (1.0 /throughput)) +
                      (_avgLatencyMsec * ((throughput - 1.0) / throughput));

    if (0 > _minLatencyMsec)
    {
        _minLatencyMsec = latencyMsec;
    }
    else if (_minLatencyMsec > latencyMsec)
    {
        _minLatencyMsec = latencyMsec;
    }

    if (latencyMsec > _maxLatencyMsec)
    {
        _maxLatencyMsec = latencyMsec;
    }

    _lock.writeRelease();
}

void AWSPerformanceCounter::printSummary() const
{
    fprintf(stdout, "%s Summary:\n", getName());

    fprintf(stdout, "\tThroughput: %lu\n", getThroughput());
    fprintf(stdout, "\tAverage Latency Msec: %f\n", getAverageLatencyMsec());
    fprintf(stdout, "\tMin Latency Msec: %f\n", getMinLatencyMsec());
    fprintf(stdout, "\tMax Latency Msec: %f\n", getMaxLatencyMsec());
}
