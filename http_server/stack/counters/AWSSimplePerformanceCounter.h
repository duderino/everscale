/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_SIMPLE_PERFORMANCE_COUNTER_H
#define AWS_SIMPLE_PERFORMANCE_COUNTER_H

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef AWS_PERFORMANCE_COUNTER_H
#include <AWSPerformanceCounter.h>
#endif

#include <sys/time.h>

class AWSSimplePerformanceCounter : public AWSPerformanceCounter {
public:
    /**
     * Create a new counter with no stop or start time.
     *
     * @param name The counter's name
     */
    AWSSimplePerformanceCounter(const char *name);

    /**
     * Create a new counter with a start and a stop time.
     *
     * @param name The counter's name
     * @param startTime the start time of the counter's window (number of seconds since the epoch)
     * @param stopTime The stop time of the counter's window (number of seconds since the epoch).
     */
    AWSSimplePerformanceCounter(const char *name, time_t startTime, time_t stopTime);

    virtual ~AWSSimplePerformanceCounter();

    virtual void addObservation(const struct timeval *start);

    virtual void addObservation(const struct timeval *start, const struct timeval *stop);

    virtual void printSummary(FILE *file) const;

    inline const char *getName() const {
        return _name;
    }

    inline time_t getStartTime() const {
        return _startTime;
    }

    inline time_t getStopTime() const {
        return _stopTime;
    }

    inline unsigned long getThroughput() const {
        return _throughput;
    }

    inline double getAverageLatencyMsec() const {
        return _avgLatencyMsec;
    }

    inline double getMinLatencyMsec() const {
        return 0 > _minLatencyMsec ? 0 : _minLatencyMsec;
    }

    inline double getMaxLatencyMsec() const {
        return _maxLatencyMsec;
    }

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
    // Disabled
    AWSSimplePerformanceCounter(const AWSSimplePerformanceCounter &counter);
    AWSSimplePerformanceCounter *operator=(const AWSSimplePerformanceCounter &counter);

    const time_t _startTime;
    const time_t _stopTime;
    const char *_name;
    double _avgLatencyMsec;
    double _minLatencyMsec;
    double _maxLatencyMsec;
    unsigned long _throughput;
    mutable ESFMutex _lock;
};

#endif
