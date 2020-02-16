/** @file ESFNullLogger.h
 *  @brief A no-op implementation of the ESFLogger interface.
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

#ifndef ESF_NULL_LOGGER_H
#define ESF_NULL_LOGGER_H

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

/** A no-op implementation of the ESFLogger interface.
 *
 *  @ingroup log
 */
class ESFNullLogger : public ESFLogger {
 public:
  /** Singleton accessor.
   */
  inline static ESFNullLogger *GetInstance() { return &_Instance; }

  /** Destructor
   */
  virtual ~ESFNullLogger();

  /** Determine whether a log message will really be logged.
   *
   * @param severity The severity of the message to be logged
   * @return true if the messages will really be logged, false otherwise.
   */
  virtual bool isLoggable(Severity severity);

  /** Set the severity level at which messages will be logged.
   *
   *  @param severity Messages with a severity greater than or equal to
   *      this severity level will be logged
   */
  virtual void setSeverity(Severity severity);

  /** Log a message.
   *
   *  @param severity The severity of the event.
   *  @param file The name of the file logging the message.
   *  @param line The line of the file that the message was logged.
   *  @param format A printf-style format string.
   *  @return ESF_SUCCESS if successful, another value otherwise.
   */
  virtual ESFError log(Severity severity, const char *file, int line,
                       const char *format, ...);

 private:
  /** Constructor
   */
  ESFNullLogger();

  // Disabled
  ESFNullLogger(const ESFNullLogger &);
  void operator=(const ESFNullLogger &);

  static ESFNullLogger _Instance;
};

#endif /* ! ESF_NULL_LOGGER_H */
