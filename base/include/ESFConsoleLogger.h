/** @file ESFConsoleLogger.h
 *  @brief An implementation of the ESFLogger interface that logs messages to
 *  the system's console.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:08 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_CONSOLE_LOGGER_H
#define ESF_CONSOLE_LOGGER_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

#ifndef ESF_ERROR_H
#include <ESFError.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

/** ESFConsoleLogger is a singleton that implements the ESFLogger interface by
 *  writing log messages to stderr.
 *
 *  @ingroup log
 */
class ESFConsoleLogger: public ESFLogger {
public:

    /** Initialize the ESFConsoleLogger
     *
     *  @param severity Messages with a severity greater than or equal to
     *      this severity level will be logged.
     *  @return ESF_SUCCESS if successful, ESF_OPERATION_NOT_SUPPORTED if this
     *      platform does support console logging.
     */
    static ESFError Initialize(Severity severity);

    /** Destroy the ESFConsoleLogger
     *
     *  @return ESF_SUCCESS if successful, ESF_OPERATION_NOT_SUPPORTED if this
     *      platform does not support console logging.
     */
    static ESFError Destroy();

    /** Get a ESFConsoleLogger instance.  This method should only be called
     *  when the ESFConsoleLogger is initialized.
     *
     *  @return the instance
     */
    static ESFConsoleLogger *Instance();

    /** Destructor
     */
    virtual ~ESFConsoleLogger();

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
    void setSeverity(Severity severity);

    /** Log a message.
     *
     *  @param severity The severity of the event.
     *  @param file The name of the file logging the message.
     *  @param line The line of the file that the message was logged.
     *  @param format A printf-style format string.
     *  @return ESF_SUCCESS if successful, ESF_NULL_POINTER if any mandatory
     *      arguments are NULL, ESF_OPERATION_NOT_SUPPORTED if this platform
     *      does not suppoort console logging.
     */
    ESFError log(Severity severity, const char *file, int line, const char *format, ...);

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

    // Singleton idiom
    ESFConsoleLogger();

    // Disabled
    ESFConsoleLogger(const ESFConsoleLogger &);
    ESFConsoleLogger &operator=(const ESFConsoleLogger &);

    static ESFConsoleLogger _Instance;

    Severity _severity;
};

#endif /* ! ESF_CONSOLE_LOGGER_H */
