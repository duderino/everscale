/** @file ESTFResultCollector.h
 *  @brief An object that collects the results of a test run.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:19 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESTF_RESULT_COLLECTOR_H
#define ESTF_RESULT_COLLECTOR_H

#ifndef ESTF_DATE_H
#include <ESTFDate.h>
#endif

#ifndef ESTF_THREAD_H
#include <ESTFThread.h>
#endif

#include <set>
#include <iostream>
#include <string>

/** ESTFResultCollector collects the results of multiple test cases across
 *    multiple test runs.  They are unsynchronized.  Concurrent test cases should
 *    each use their own ESTFResultCollector - they can always be merged back
 *    into a single collector at the end of the test run.
 *
 *  @ingroup test
 */
class ESTFResultCollector
{
public:

    /** Default constructor. */
    ESTFResultCollector();

    /** Destructor. */
    virtual ~ESTFResultCollector();

    /** Add a success result of a test case to a result collector.
     *
     *    @param description The result description (e.g., "x == 12").
     *    @param file The file (e.g., "my_test.cpp").
     *    @param line The line of the test (e.g., 1231).
     */
    void addSuccess( const char *description, const char *file, int line );

    /** Add a failure of a test case to a result collector.  A failure is
     *    defined as a test performed by a test case that did not pass.
     *
     *  @param description The result description (e.g., "x == 12").
     *  @param file The file (e.g., "my_test.cpp").
     *  @param line The line of the test (e.g., 1231).
     */
    void addFailure( const char *description, const char *file, int line );

    /** Add an error condition to a result collector.  A error is defined as
      *    a error condition that occurred outside of a test.  A failure of the
     *    test framework itself, for instance, would be reported as an error.
     *
     *  @param description The result description (e.g., "x == 12").
     *  @param file The file (e.g., "my_test.cpp").
     *  @param line The line of the test (e.g., 1231).
     */
    void addError( const char *description, const char *file, int line );

    /** Merges the results of another result collector into this collector.
     *
      *    @param collector The other result collector.
     */
    void merge( const ESTFResultCollector *collector );

    /** Print the results contain in this file to an output stream.
     *
     *    @param output The output stream.
     *    @return The output stream (for chaining).
     */
    ostream &print( ostream &output ) const;

    /** Return the number of Successes recorded by this collector.
     *
     *    @return The number of Success Results.
     */
    inline int getSuccessCount()
    {
        return _successes;
    }

    /** Return the number of Failures recorded by this collector.
     *
     *  @return The number of Failure Results.
     */
    inline int getFailureCount()
    {
        return _failures;
    }

    /** Return the number of Successes recorded by this collector.
     *
     *  @return The number of Success Results.
     */
    inline int getErrorCount()
    {
        return _errors;
    }

    /** @todo maybe add the ability to iterate through the result set? */

    /** Defines the category of a test case's test result. */
    typedef enum
    {
        Success = 0, /**< Success. */
        Failure = 1, /**< Anticipated failure. */
        Error = 2    /**< Unanticipated failure. */
    } ESTFResultType;

    /** Carries the result of a test case.  Note that a single test case can
     *  produce multiple results.
     */
    class ESTFResult
    {
    public:

        /** Constructor.
         *
         *  @param type The result type (i.e., Success, Failure, Error)
         *  @param description A description of the test performed.
         *  @param file The file where the test occured.
         *  @param line The line in the file where the test occured.
         *  @param threadId The thread that performed the test.
         *  @param date The time the result was recorded.
         */
        ESTFResult( ESTFResultType type, const char *description,
            const char *file, int line, ESTFThread::ThreadId threadId,
            ESTFDate date );

        /** Destructor. */
        virtual ~ESTFResult();

        /** Less than comparitor for two ESTFResult instances.  ESTFResults
         *    order themselves by their dates.
          *
          *    @param result The other instance.
          *    @return true if this instance is logically less than the other
         *        instance, false otherwise.
          */
        inline bool operator<( const ESTFResult &result ) const
        {
            return _date < result._date;
        }

        /** Print the results contained in this result record to an output
         *    stream.
         *
         *    @param output The output stream.
         *    @return The output stream (for chaining).
         */
        ostream &print( ostream &output ) const;

        /** Get the type of the result (i.e., Success, Failure, or Error).
         *
         *    @return The type of the result.
         */
        inline ESTFResultType getType() const
        {
            return _type;
        }

    private:

        ESTFResultType _type;
        std::string _description;
        const char *_file;
        int _line;
        ESTFThread::ThreadId _threadId;
        ESTFDate _date;
    };

private:

    void add( ESTFResultType type, const char *description, const char *file,
        int line );

    multiset<ESTFResult>    _results;

    int _successes;
    int _failures;
    int _errors;
};

/** Extractor function for ESTFResultCollector.
 *
 *    @param out The output stream.
 *    @param collector The collector to print.
 *    @return The output stream (for chaining).
 *  @ingroup test
 */
inline ostream &operator<<( ostream &out, const ESTFResultCollector &collector )
{
    return collector.print( out );
}

/** Extractor function for ESTFResultCollector::ESTFResult.
 *
 *  @param out The output stream.
 *  @param result The result to print.
 *  @return The output stream (for chaining).
 *  @ingroup test
 */
inline ostream &operator<<( ostream &out,
                            const ESTFResultCollector::ESTFResult &result )
{
    return result.print( out );
}

#endif                                 /* ! ESTF_RESULT_COLLECTOR_H */
