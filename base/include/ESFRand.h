/** @file ESFRand.h
 *  @brief A pseudo-random number generator
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *  $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_RAND_H
#define ESF_RAND_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

/** ESFRand generates pseudorandom numbers using the C Standard Library or
 *  POSIX.1 random number generator.
 *
 *  @ingroup util
 */
class ESFRand {
public:

    /** Default constructor.  The seed is initialized to the current system
     *  time.
     */
    ESFRand();

    /** Constructor.
     *
     * @param seed The seed for the random number generator
     */
    ESFRand(unsigned int seed);

    /** Default destructor. */
    virtual ~ESFRand();

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
    inline void *operator new(size_t size, ESFAllocator *allocator) {
        return allocator->allocate(size);
    }

private:

#ifdef HAVE_RAND_R
    unsigned int _seed;
#endif

};

#endif /* ! ESF_RAND_H */
