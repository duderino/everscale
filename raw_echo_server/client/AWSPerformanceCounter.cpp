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
_throughput(0UL)
{
    pthread_mutex_init(&_lock, 0);
}


AWSPerformanceCounter::~AWSPerformanceCounter()
{
    pthread_mutex_destroy(&_lock);
}

void AWSPerformanceCounter::GetTime(struct timeval *now)
{
    gettimeofday(now, 0);
}

void AWSPerformanceCounter::addObservation(struct timeval *start)
{
    struct timeval now;

    gettimeofday(&now, 0);

    double latencyMsec = (now.tv_sec - start->tv_sec) * 1000.0;
    latencyMsec += now.tv_usec / 1000.0;
    latencyMsec -= start->tv_usec / 1000.0;

    pthread_mutex_lock(&_lock);

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

    pthread_mutex_unlock(&_lock);
}

void AWSPerformanceCounter::printSummary()
{
    fprintf(stdout, "%s Summary:\n", getName());

    fprintf(stdout, "\tThroughput: %lu\n", getThroughput());
    fprintf(stdout, "\tAverage Latency Msec: %f\n", getAverageLatencyMsec());
    fprintf(stdout, "\tMin Latency Msec: %f\n", getMinLatencyMsec());
    fprintf(stdout, "\tMax Latency Msec: %f\n", getMaxLatencyMsec());
}
