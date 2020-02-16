/** @file ESFDate.h
 *  @brief A microsecond-resolution clock
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under
 * both BSD and Apache 2.0 licenses
 * (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_DATE_H
#define ESF_DATE_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_TYPES_H
#include <ESFTypes.h>
#endif

/** @defgroup util Utilities */

/** ESFDate is a microsecond resolution clock
 *
 *  @ingroup util
 */
class ESFDate {
 public:
  /** Default constructor.  The value of an object initialized by this
   *  constructor is 0, not the current time.
   *
   */
  inline ESFDate() : _seconds(0), _microSeconds(0) {}

  /** Constructor.
   *
   *    @param seconds Set initial seconds value of this date object to seconds.
   *    @param microSeconds Set initial microSeconds value of this date object
   *        to microSeconds.
   */
  inline ESFDate(ESFUInt32 seconds, ESFUInt32 microSeconds)
      : _seconds(seconds), _microSeconds(microSeconds) {}

  /** Copy constructor.
   *
   *    @param date The date to copy.
   */
  inline ESFDate(const ESFDate &date) {
    _seconds = date._seconds;
    _microSeconds = date._microSeconds;
  }

  /** Default destructor. */
  virtual ~ESFDate();

  /** Get seconds.
   *
   *    @return The seconds field of this date object.
   */
  inline ESFUInt32 getSeconds() const { return _seconds; }

  /** Set seconds.
   *
   *    @param seconds Set the seconds field of this object to this param.
   */
  inline void setSeconds(ESFUInt32 seconds) { _seconds = seconds; }

  /** Get micro seconds.
   *
   *    @return The microSeconds field of this date object.
   */
  inline ESFUInt32 getMicroSeconds() const { return _microSeconds; }

  /** Set micro seconds.  Will modify seconds field if new value is not
   *    less than one second.
   *
   *    @param microSeconds Set the microSeconds field of this object to this
   *        param.
   */
  inline void setMicroSeconds(ESFUInt32 microSeconds) {
    _seconds += microSeconds / ESF_UINT32_C(1000000);
    _microSeconds = microSeconds % ESF_UINT32_C(1000000);
  }

  /** Assignment operator.
   *
   *    @param date The date to copy.
   *    @return this object.
   */
  inline ESFDate &operator=(const ESFDate &date) {
    _seconds = date._seconds;
    _microSeconds = date._microSeconds;

    return *this;
  }

  /** Assignment operator.  This operation only supports second resolution.
   *
   *  @param seconds The seconds value to assign to this date.  This
   *      object's microsecond attribute will be set to zero.
   *  @return this object.
   */
  inline ESFDate &operator=(ESFUInt32 seconds) {
    _seconds = seconds;
    _microSeconds = 0;

    return *this;
  }

  /** Plus equals operator.
   *
   *    @param date The date to add to this date.
   *    @return this object.
   */
  ESFDate &operator+=(const ESFDate &date);

  /** Plus equals operator.  This operation only supports second resolution.
   *
   *  @param seconds The seconds value to add to this date.  The microseconds
   *      attribute is unaffected by this operation.
   *  @return this object.
   */
  inline ESFDate &operator+=(ESFUInt32 seconds) {
    _seconds += seconds;

    return *this;
  }

  /** Minus equals operator.
   *
   *    @param date The date to subtract from this date.
   *    @return this object.
   */
  ESFDate &operator-=(const ESFDate &date);

  /** Minus equals operator.  This operation only supports second resolution.
   *
   *  @param seconds The seconds value subtract from this date.  The
   *      microsecondds attribute is unaffected by this operation.
   *  @return this object
   */
  inline ESFDate &operator-=(ESFUInt32 seconds) {
    _seconds -= seconds;

    return *this;
  }

  /** Unary plus operator.
   *
   *    @param date The date to add to this one.
   *    @return The sum of the date param and this object.
   */
  inline ESFDate &operator+(const ESFDate &date) const {
    ESFDate newDate(*this);

    return newDate += date;
  }

  /** Unary plus operator.  This operation only supports second resolution.
   *
   *  @param seconds The seconds value to add to this date.
   *  @return The sum of the seconds param and this object.
   */
  inline ESFDate &operator+(ESFUInt32 seconds) const {
    ESFDate newDate(*this);

    return newDate += seconds;
  }

  /** Unary minus operator.
   *
   *    @param date The date to subtract from this one.
   *    @return The difference of the date param and this object.
   */
  inline ESFDate &operator-(const ESFDate &date) const {
    ESFDate newDate(*this);

    return newDate -= date;
  }

  /** Unary minus operation.  This operation only supports seconds resolution.
   *
   *  @param seconds The seconds value to add to this date.
   *  @return The difference of the seconds param and this object.
   */
  inline ESFDate &operator-(ESFUInt32 seconds) const {
    ESFDate newDate(*this);

    return newDate -= seconds;
  }

  /** Less than operator.
   *
   *    @param date The date to compare to this object.
   *    @return true if this object is less than the date param, false
   *        otherwise.
   */
  inline bool operator<(const ESFDate &date) const {
    if (_seconds < date._seconds) return true;

    if (_seconds > date._seconds) return false;

    return _microSeconds < date._microSeconds;
  }

  /** Greater than or equal to operator.
   *
   *  @param date The date to compare to this object.
   *  @return true if this object is less than or equal to the date param,
   *        false otherwise.
   */
  inline bool operator<=(const ESFDate &date) const {
    if (_seconds < date._seconds) return true;

    if (_seconds > date._seconds) return false;

    return _microSeconds <= date._microSeconds;
  }

  /** Greater than operator.
   *
   *    @param date The date to compare to this object.
   *    @return true if this object is greater than the date param, false
   *        otherwise.
   */
  inline bool operator>(const ESFDate &date) const {
    if (_seconds > date._seconds) return true;

    if (_seconds < date._seconds) return false;

    return _microSeconds > date._microSeconds;
  }

  /** Greater than or equal to operator.
   *
   *    @param date The date to compare to this object.
   *    @return true if this object is greater than or equal to the date
   *        param, false otherwise.
   */
  inline bool operator>=(const ESFDate &date) const {
    if (_seconds > date._seconds) return true;

    if (_seconds < date._seconds) return false;

    return _microSeconds >= date._microSeconds;
  }

  /** Equality operator.
   *
   *    @param date The date to compare to this object.
   *  @return true if both objects are logically equal, false otherwise.
   */
  inline bool operator==(const ESFDate &date) const {
    if (_seconds != date._seconds) return false;

    return _microSeconds == date._microSeconds;
  }

  /** Inequality operator.
   *
   *    @param date The date to compare to this object.
   *  @return true if both objects are distinct, false otherwise.
   */
  inline bool operator!=(const ESFDate &date) const {
    if (_seconds != date._seconds) return true;

    return _microSeconds != date._microSeconds;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

  /** Get a new date object initialized to the current system time.
   *
   *    @return date object set to the current time.
   */
  static ESFDate GetSystemTime();

 private:
  ESFUInt32 _seconds;
  ESFUInt32 _microSeconds;
};

#endif /* ! ESF_DATE_H */
