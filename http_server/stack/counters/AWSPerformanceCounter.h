/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_PERFORMANCE_COUNTER_H
#define AWS_PERFORMANCE_COUNTER_H

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#include <sys/time.h>

class AWSPerformanceCounter
{
public:
    AWSPerformanceCounter(const char *name);

    virtual ~AWSPerformanceCounter();

    static void GetTime(struct timeval *now);

    void addObservation(const struct timeval *start);

    inline const char *getName() const
    {
        return _name;
    }

    inline unsigned long getThroughput() const
    {
        return _throughput;
    }

    inline double getAverageLatencyMsec() const
    {
        return _avgLatencyMsec;
    }

    inline double getMinLatencyMsec() const
    {
        return 0 > _minLatencyMsec ? 0 : _minLatencyMsec;
    }

    inline double getMaxLatencyMsec() const
    {
        return _maxLatencyMsec;
    }

    void printSummary() const;

private:
    // Disabled
    AWSPerformanceCounter(const AWSPerformanceCounter &counter);
    AWSPerformanceCounter *operator=(const AWSPerformanceCounter &counter);

    const char *_name;
    double _avgLatencyMsec;
    double _minLatencyMsec;
    double _maxLatencyMsec;
    unsigned long _throughput;
    ESFMutex _lock;
};

#endif
