#ifndef ESB_RAND_H
#define ESB_RAND_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {

/** Rand generates pseudorandom numbers using the C Standard Library or
 *  POSIX.1 random number generator.
 *
 *  @ingroup util
 */
class Rand {
 public:
  /** Default constructor.  The seed is initialized to the current system
   *  time.
   */
  Rand();

  /** Constructor.
   *
   * @param seed The seed for the random number generator
   */
  Rand(unsigned int seed);

  /** Default destructor. */
  virtual ~Rand();

  /** Generate a random number between 0.0 inclusive and 1.0 exclusive.
   *
   *    @return A random number.
   */
  double generateRandom();

  /** Generate a random integer within a given range.
   *
   *    @param lowerBound The lowerbound of the range.  Inclusive.
   *    @param upperBound The upperbound of the range.  Inclusive.
   *    @return A random number within the given range.
   */
  int generateRandom(int lowerBound, int upperBound);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator *allocator) {
    return allocator->allocate(size);
  }

 private:
#ifdef HAVE_RAND_R
  unsigned int _seed;
#endif
};

}  // namespace ESB

#endif
