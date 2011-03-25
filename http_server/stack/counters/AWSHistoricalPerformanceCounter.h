/* Copyright (c) 2011 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 */

#ifndef AWS_HISTORICAL_PERFORMANCE_COUNTER_H
#define AWS_HISTORICAL_PERFORMANCE_COUNTER_H

#ifndef AWS_SIMPLE_PERFORMANCE_COUNTER_H
#include <AWSSimplePerformanceCounter.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

class AWSHistoricalPerformanceCounter : public AWSPerformanceCounter {
public:
    /**
     * Constructor.
     *
     * @param name The name of the counter
     * @param windowSizeSec The counter's latency and throughput will be saved
     *   for each window of this many seconds.
     * @param allocator The counter will grab memory from this allocator
     *   for each new window it creates.
     */
    AWSHistoricalPerformanceCounter(const char *name, time_t windowSizeSec, ESFAllocator *allocator, ESFLogger *logger);

    virtual ~AWSHistoricalPerformanceCounter();

    virtual void addObservation(const struct timeval *start);

    virtual void addObservation(const struct timeval *start, const struct timeval *stop);

    inline const char *getName() const {
        return _name;
    }

    /**
     * Get a list of AWSSimplePerformanceCounters.  There will be one counter in the
     * list per window that actually had observations.
     *
     * @return The list of AWSSimplePerformanceCounters.
     */
    inline const ESFEmbeddedList *getCounters() const {
        return &_list;
    }

    virtual void printSummary(FILE *file) const;

private:
    // Disabled
    AWSHistoricalPerformanceCounter(const AWSHistoricalPerformanceCounter &counter);
    AWSHistoricalPerformanceCounter *operator=(const AWSHistoricalPerformanceCounter &counter);

    time_t _windowSizeSec;
    const char *_name;
    ESFLogger *_logger;
    ESFEmbeddedList _list;
    mutable ESFMutex _lock;
    ESFDiscardAllocator _allocator;
};

#endif
