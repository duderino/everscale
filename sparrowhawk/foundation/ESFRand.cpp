/** @file ESFRand.cpp
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
#include "ESFRand.h"
#endif

#ifndef ESF_DATE_H
#include <ESFDate.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/** @todo for win32, seed the global random number seed with
 *  ESFDate::GetSystemTime().getSeconds()
 */

ESFRand::ESFRand() {
#ifdef HAVE_RAND_R
    _seed = ESFDate::GetSystemTime().getSeconds();
#endif
}

ESFRand::ESFRand(unsigned int seed) :
    _seed(seed) {
}

ESFRand::~ESFRand() {
}

double ESFRand::generateRandom() {
#if defined HAVE_RAND_R && defined HAVE_DECL_RAND_MAX
    return rand_r(&_seed) / (RAND_MAX + 1.0);
#elif defined HAVE_RAND && defined HAVE_DECL_RAND_MAX
    return rand() / ( RAND_MAX + 1.0 );
#else
#error "rand_r and RAND_MAX or equivalent is required"
#endif
}

int ESFRand::generateRandom(int lowerBound, int upperBound) {
#if defined HAVE_RAND_R && defined HAVE_DECL_RAND_MAX
    return lowerBound + (int) ((upperBound - lowerBound + 1.0) * rand_r(&_seed) / (RAND_MAX + 1.0));
#elif defined HAVE_RAND && defined HAVE_DECL_RAND_MAX
    return lowerBound + ( int ) ( ( upperBound - lowerBound + 1.0 ) *
            rand() / ( RAND_MAX + 1.0 ) );
#else
#error "rand_r and RAND_MAX or equivalent is required"
#endif
}
