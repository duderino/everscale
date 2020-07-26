#ifndef ESB_DATE_H
#define ESB_DATE_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_TYPES_H
#include <ESBTypes.h>
#endif

namespace ESB {

/** @defgroup util Utilities */

/** Date is a microsecond resolution clock
 *
 *  @ingroup util
 */
class Date {
 public:
  /** Default constructor.  The value of an object initialized by this
   *  constructor is 0, not the current time.
   *
   */
  inline Date() : _seconds(0), _microSeconds(0) {}

  /** Constructor.
   *
   *    @param seconds Set initial seconds value of this date object to seconds.
   *    @param microSeconds Set initial microSeconds value of this date object
   *        to microSeconds.
   */
  inline Date(UInt32 seconds, UInt32 microSeconds) : _seconds(seconds), _microSeconds(microSeconds) {}

  /** Copy constructor.
   *
   *    @param date The date to copy.
   */
  inline Date(const Date &date) {
    _seconds = date._seconds;
    _microSeconds = date._microSeconds;
  }

  /** Default destructor. */
  virtual ~Date();

  /** Get seconds.
   *
   *    @return The seconds field of this date object.
   */
  inline UInt32 seconds() const { return _seconds; }

  /** Set seconds.
   *
   *    @param seconds Set the seconds field of this object to this param.
   */
  inline void setSeconds(UInt32 seconds) { _seconds = seconds; }

  /** Get micro seconds.
   *
   *    @return The microSeconds field of this date object.
   */
  inline UInt32 microSeconds() const { return _microSeconds; }

  /** Set micro seconds.  Will modify seconds field if new value is not
   *    less than one second.
   *
   *    @param microSeconds Set the microSeconds field of this object to this
   *        param.
   */
  inline void setMicroSeconds(UInt32 microSeconds) {
    _seconds += microSeconds / ESB_UINT32_C(1000000);
    _microSeconds = microSeconds % ESB_UINT32_C(1000000);
  }

  /** Assignment operator.
   *
   *    @param date The date to copy.
   *    @return this object.
   */
  inline Date &operator=(const Date &date) {
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
  inline Date &operator=(UInt32 seconds) {
    _seconds = seconds;
    _microSeconds = 0;

    return *this;
  }

  /** Plus equals operator.
   *
   *    @param date The date to add to this date.
   *    @return this object.
   */
  Date &operator+=(const Date &date);

  /** Plus equals operator.  This operation only supports second resolution.
   *
   *  @param seconds The seconds value to add to this date.  The microseconds
   *      attribute is unaffected by this operation.
   *  @return this object.
   */
  inline Date &operator+=(UInt32 seconds) {
    _seconds += seconds;

    return *this;
  }

  /** Minus equals operator.
   *
   *    @param date The date to subtract from this date.
   *    @return this object.
   */
  Date &operator-=(const Date &date);

  /** Minus equals operator.  This operation only supports second resolution.
   *
   *  @param seconds The seconds value subtract from this date.  The
   *      microsecondds attribute is unaffected by this operation.
   *  @return this object
   */
  inline Date &operator-=(UInt32 seconds) {
    _seconds -= seconds;

    return *this;
  }

  /** Unary plus operator.
   *
   *    @param date The date to add to this one.
   *    @return The sum of the date param and this object.
   */
  inline Date operator+(const Date &date) const {
    Date newDate(*this);

    return newDate += date;
  }

  /** Unary plus operator.  This operation only supports second resolution.
   *
   *  @param seconds The seconds value to add to this date.
   *  @return The sum of the seconds param and this object.
   */
  inline Date operator+(UInt32 seconds) const {
    Date newDate(*this);

    return newDate += seconds;
  }

  /** Unary minus operator.
   *
   *    @param date The date to subtract from this one.
   *    @return The difference of the date param and this object.
   */
  inline Date operator-(const Date &date) const {
    Date newDate(*this);

    return newDate -= date;
  }

  /** Unary minus operation.  This operation only supports seconds resolution.
   *
   *  @param seconds The seconds value to add to this date.
   *  @return The difference of the seconds param and this object.
   */
  inline Date operator-(UInt32 seconds) const {
    Date newDate(*this);

    return newDate -= seconds;
  }

  /** Less than operator.
   *
   *    @param date The date to compare to this object.
   *    @return true if this object is less than the date param, false
   *        otherwise.
   */
  inline bool operator<(const Date &date) const {
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
  inline bool operator<=(const Date &date) const {
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
  inline bool operator>(const Date &date) const {
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
  inline bool operator>=(const Date &date) const {
    if (_seconds > date._seconds) return true;

    if (_seconds < date._seconds) return false;

    return _microSeconds >= date._microSeconds;
  }

  /** Equality operator.
   *
   *    @param date The date to compare to this object.
   *  @return true if both objects are logically equal, false otherwise.
   */
  inline bool operator==(const Date &date) const {
    if (_seconds != date._seconds) return false;

    return _microSeconds == date._microSeconds;
  }

  /** Inequality operator.
   *
   *    @param date The date to compare to this object.
   *  @return true if both objects are distinct, false otherwise.
   */
  inline bool operator!=(const Date &date) const {
    if (_seconds != date._seconds) return true;

    return _microSeconds != date._microSeconds;
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return The new object or NULL of the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept { return allocator.allocate(size); }

 private:
  UInt32 _seconds;
  UInt32 _microSeconds;
};

}  // namespace ESB

#endif
