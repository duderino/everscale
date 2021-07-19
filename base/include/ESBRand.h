#ifndef ESB_RAND_H
#define ESB_RAND_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
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
  double generate();

  /** Generate a random integer within a given range.
   *
   *    @param lowerBound The lowerbound of the range.  Inclusive.
   *    @param upperBound The upperbound of the range.  Inclusive.
   *    @return A random number within the given range.
   */
  inline Int32 generate(Int32 lowerBound, Int32 upperBound) {
    assert(lowerBound < upperBound);
    return lowerBound + ((Int64)upperBound - lowerBound + 1.0) * generate();
  }

  /** Generate a random integer within a given range.
   *
   *    @param lowerBound The lowerbound of the range.  Inclusive.
   *    @param upperBound The upperbound of the range.  Inclusive.
   *    @return A random number within the given range.
   */
  inline UInt32 generate(UInt32 lowerBound, UInt32 upperBound) {
    assert(lowerBound < upperBound);
    return lowerBound + ((UInt64)upperBound - lowerBound + 1.0) * generate();
  }

 private:
#ifdef HAVE_RAND_R
  unsigned int _seed;
#endif

  ESB_DEFAULT_FUNCS(Rand);
};

}  // namespace ESB

#endif
