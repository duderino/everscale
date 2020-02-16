/** @file ESTFDate.cpp
 *  @brief A microsecond resolution clock
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 */

#ifndef ESTF_DATE_H
#include "ESTFDate.h"
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

namespace ESTF {

Date::~Date()
{
}

Date &Date::operator+=( const Date &date )
{
    _seconds += date._seconds;

    _seconds += ( _microSeconds + date._microSeconds ) / 1000000L;
    _microSeconds = ( _microSeconds + date._microSeconds ) % 1000000L;

    return *this;
}

Date &Date::operator-=( const Date &date )
{
    _seconds -= date._seconds;
    _microSeconds -= date._microSeconds;

    if ( 0 > _microSeconds )
    {
        --_seconds;
        _microSeconds += 1000000L;
    }

    return *this;
}

Date Date::GetSystemTime()
{
    Date currentTime;

#if defined HAVE_GETTIMEOFDAY && defined HAVE_STRUCT_TIMEVAL
    struct timeval tv;

    gettimeofday( &tv, 0 );

    currentTime.setSeconds( tv.tv_sec );
    currentTime.setMicroSeconds( tv.tv_usec );
#else
#error "gettimeofday or equivalent is required"
#endif

    return currentTime;
}

std::ostream &Date::print( std::ostream &output ) const
{
    output << _seconds << "." << _microSeconds;

    /** @todo add date formatting */
    return output;
}

}
