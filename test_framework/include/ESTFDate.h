/** @file ESTFDate.h
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
#define ESTF_DATE_H

#ifndef ESTF_CONFIG_H
#include <ESTFConfig.h>
#endif

#include <iostream>

namespace ESTF {

/** Date is a simple clock that allows up to microsecond resolution.  That
 *    is, this clock will always return dates specified in seconds and
 *    microSeconds, but the actual resolution may be less depending on platform.
 *
 *  @ingroup test
 */
class Date
{
public:

    /** Default constructor. */
    inline Date() : _seconds(0), _microSeconds(0)
    {
    }

    /** Constructor.
     *
     *    @param seconds Set initial seconds value of this date object to seconds.
     *    @param microSeconds Set initial microSeconds value of this date object
     *        to microSeconds.
     */
    inline Date( long seconds, long microSeconds ) : _seconds(seconds),
        _microSeconds(microSeconds)
    {
    }

    /** Copy constructor.
     *
      *    @param date The date to copy.
     */
    inline Date( const Date &date )
    {
        _seconds = date._seconds;
        _microSeconds = date._microSeconds;
    }

    /** Default destructor. */
    virtual ~Date();

    /** Get seconds.
     *
     *    @return The seconds field of this date object.
     */
    inline long getSeconds()
    {
        return _seconds;
    }

    /** Set seconds.
     *
     *    @param seconds Set the seconds field of this object to this param.
     */
    inline void setSeconds( long seconds )
    {
        _seconds = seconds;
    }

    /** Get micro seconds.
     *
     *    @return The microSeconds field of this date object.
     */
    inline long getMicroSeconds()
    {
        return _microSeconds;
    }

    /** Set micro seconds.  Will modify seconds field if new value is not
     *    less than one second.
     *
     *    @param microSeconds Set the microSeconds field of this object to this
     *        param.
     */
    inline void setMicroSeconds( long microSeconds )
    {
        _seconds += microSeconds / 1000000L;
        _microSeconds = microSeconds % 1000000L;
    }

    /** Assignment operator.
     *
     *    @param date The date to copy.
     *    @return this object.
     */
    inline Date &operator=( const Date &date )
    {
        _seconds = date._seconds;
        _microSeconds = date._microSeconds;

        return *this;
    }

    /** Plus equals operator.
     *
     *    @param date The date to add to this date.
     *    @return this object.
     */
    Date &operator+=( const Date &date );

    /** Minus equals operator.
     *
     *    @param date The date to subtract from this date.
     *    @return this object.
     */
    Date &operator-=( const Date &date );

    /** Unary plus operator.
     *
     *    @param date The date to add to this one.
     *    @return The sum of the date param and this object.
     */
    inline Date &operator+( const Date &date ) const
    {
        Date newDate( *this );

        return newDate += date;
    }

    /** Unary minus operator.
     *
     *    @param date The date to subtract from this one.
     *    @return The difference of the date param and this object.
     */
    inline Date &operator-( const Date &date ) const
    {
        Date newDate( *this );

        return newDate -= date;
    }

    /** Less than operator.
     *
     *    @param date The date to compare to this object.
     *    @return true if this object is less than the date param, false
     *        otherwise.
     */
    inline bool operator<( const Date &date ) const
    {
        if ( _seconds < date._seconds ) return true;

        if ( _seconds > date._seconds ) return false;

        return _microSeconds < date._microSeconds;
    }

    /** Greater than or equal to operator.
     *
     *  @param date The date to compare to this object.
     *  @return true if this object is less than or equal to the date param,
     *        false otherwise.
     */
    inline bool operator<=( const Date &date ) const
    {
        if ( _seconds < date._seconds ) return true;

        if ( _seconds > date._seconds ) return false;

        return _microSeconds <= date._microSeconds;
    }

    /** Greater than operator.
     *
     *    @param date The date to compare to this object.
     *    @return true if this object is greater than the date param, false
     *        otherwise.
     */
    inline bool operator>( const Date &date ) const
    {
        if ( _seconds > date._seconds ) return true;

        if ( _seconds < date._seconds ) return false;

        return _microSeconds > date._microSeconds;
    }

    /** Greater than or equal to operator.
     *
     *    @param date The date to compare to this object.
     *    @return true if this object is greater than or equal to the date
     *        param, false otherwise.
     */
    inline bool operator>=( const Date &date ) const
    {
        if ( _seconds > date._seconds ) return true;

        if ( _seconds < date._seconds ) return false;

        return _microSeconds >= date._microSeconds;
    }

    /** Equality operator.
     *
     *    @param date The date to compare to this object.
     *  @return true if both objects are logically equal, false otherwise.
     */
    inline bool operator==( const Date &date ) const
    {
        if ( _seconds != date._seconds ) return false;

        return _microSeconds == date._microSeconds;
    }

    /** Inequality operator.
     *
     *    @param date The date to compare to this object.
     *  @return true if both objects are distinct, false otherwise.
     */
    inline bool operator!=( const Date &date ) const
    {
        if ( _seconds != date._seconds ) return true;

        return _microSeconds != date._microSeconds;
    }

    /** Get a new date object initialized to the current system time.
     *
     *    @return date object set to the current time.
     */
    static Date GetSystemTime();

    /** Pretty print the time represented by this Date to an output stream.
     *
     *  @param output The output stream.
     *  @return The output stream (for chaining).
     */
    std::ostream &print( std::ostream &output ) const;

private:

    long _seconds;
    long _microSeconds;
};

/** Extractor function for Date.
 *
 *  @param out The output stream.
 *  @param date The date to print.
 *  @return The output stream (for chaining).
 *  @ingroup test
 */
inline std::ostream &operator<<( std::ostream &out, const Date &date )
{
    return date.print( out );
}

}

#endif
