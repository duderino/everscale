/** @file ESTFRand.h
 *  @param A pseudo-random number generator
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 */

#ifndef ESTF_RAND_H
#define ESTF_RAND_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#ifndef ESTF_DATE_H
#include <ESTFDate.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ESTF {

/** Rand generates pseudorandom numbers using the C Standard Library or
 *  POSIX.1 random number generator.
 */
class Rand
{
public:

    /** Default constructor. */
    Rand()
    {
        _seed = Date::GetSystemTime().getSeconds();
    }

    Rand(unsigned int seed) : _seed(seed)
    {
    }

    /** Default destructor. */
    inline ~Rand()
    {
    }

    /** Generate a random number between 0.0 inclusive and 1.0 exclusive.
     *
     *    @return A random number.
     */
    inline double generateRandom()
    {
#if defined HAVE_RAND_R && defined HAVE_DECL_RAND_MAX
        return rand_r( &_seed ) / ( RAND_MAX + 1.0 );
#else
#error "A random number generator is required"
#endif
    }

    /** Generate a random integer within a given range.
     *
     *    @param lowerBound The lowerbound of the range.  Inclusive.
     *    @param upperBound The upperbound of the range.  Inclusive.
     *    @return A random number within the given range.
     */
    inline int generateRandom( int lowerBound, int upperBound )
    {
#if defined HAVE_RAND_R && defined HAVE_DECL_RAND_MAX
        return lowerBound + ( int )
            ( ( upperBound - lowerBound + 1.0 ) * rand_r( &_seed ) /
              ( RAND_MAX + 1.0 ) );
#else
#error "A random number generator is required"
#endif
    }

private:

    unsigned int _seed;
};

}

#endif
