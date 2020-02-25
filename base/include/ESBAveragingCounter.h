#ifndef ESB_AVERAGING_COUNTER_H
#define ESB_AVERAGING_COUNTER_H

#ifndef ESB_MUTEX_H
#include <ESBMutex.h>
#endif

namespace ESB {

class AveragingCounter {
 public:
  AveragingCounter();

  virtual ~AveragingCounter();

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
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  AveragingCounter(const AveragingCounter &counter);
  void operator=(const AveragingCounter &counter);

  double _value;
  double _observations;
  Mutex _lock;
};

}  // namespace ESB

#endif
