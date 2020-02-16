/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_AVERAGING_COUNTER_H
#define AWS_AVERAGING_COUNTER_H

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

class AWSAveragingCounter {
 public:
  AWSAveragingCounter();

  virtual ~AWSAveragingCounter();

  inline void setValue(double value, double observations) {
    _observations = observations;
    _value = value;
  }

  void addValue(double value);

  inline double getValue() const { return _value; }

  inline double getObservations() const { return _observations; }

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
  AWSAveragingCounter(const AWSAveragingCounter &counter);
  void operator=(const AWSAveragingCounter &counter);

  double _value;
  double _observations;
  ESFMutex _lock;
};

#endif
