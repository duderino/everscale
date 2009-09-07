/** @file ESTFResultCollector.cpp
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
#include "ESTFResultCollector.h"
#endif

/** Default constructor. */
ESTFResultCollector::ESTFResultCollector() : _results(), _successes( 0 ),
    _failures( 0 ), _errors( 0 )
{
}

/** Destructor. */
ESTFResultCollector::~ESTFResultCollector()
{
    _results.clear();
}

void
ESTFResultCollector::add( ESTFResultType type, const char *description,
    const char *file, int line )
{
    ESTFResult result( type, description, file, line,
        ESTFThread::GetThreadId(), ESTFDate::GetSystemTime() );

    _results.insert( result );
}

/** Add a success result of a test case to a result collector.
 *
 *    @param description The result description (e.g., "x == 12").
 *    @param file The file (e.g., "my_test.cpp").
 *    @param line The line of the test (e.g., 1231).
 */
void
ESTFResultCollector::addSuccess( const char *description, const char *file,
    int line )
{
    //add( Success, description, file, line );
    ++_successes;
}

/** Add a failure of a test case to a result collector.  A failure is
 *    defined as a test performed by a test case that did not pass.
 *
 *  @param description The result description (e.g., "x == 12").
 *  @param file The file (e.g., "my_test.cpp").
 *  @param line The line of the test (e.g., 1231).
 */
void
ESTFResultCollector::addFailure( const char *description, const char *file,
    int line )
{
    add( Failure, description, file, line );
    ++_failures;
}

/** Add an error condition to a result collector.  A error is defined as
 *    a error condition that occurred outside of a test.  A failure of the
 *    test framework itself, for instance, would be reported as an error.
 *
 *  @param description The result description (e.g., "x == 12").
 *  @param file The file (e.g., "my_test.cpp").
 *  @param line The line of the test (e.g., 1231).
 */
void
ESTFResultCollector::addError( const char *description, const char *file,
    int line )
{
    add( Error, description, file, line );
    ++_errors;
}

/** Merges the results of another result collector into this collector.
 *
 *    @param collector The other result collector.
 */
void
ESTFResultCollector::merge( const ESTFResultCollector *collector )
{
    std::multiset<ESTFResult>::iterator it = collector->_results.begin();
    std::multiset<ESTFResult>::iterator end = collector->_results.end();

    for ( ; it != end; ++it )
    {
        _results.insert( *it );

        switch ( it->getType() )
        {
            case Success:
                ++_successes;
                break;
            case Failure:
                ++_failures;
                break;
            case Error:
                ++_errors;
                break;
        }
    }
}

/** Print the results contain in this file to an output stream.
 *
 *    @param output The output stream.
 *    @return The output stream (for chaining).
 */
ostream &
ESTFResultCollector::print( ostream &output ) const
{
    /** @todo print header. */

    multiset<ESTFResult>::iterator it = _results.begin();
    multiset<ESTFResult>::iterator end = _results.end();

    for( ; it != end; ++it )
    {
        output << *it << '\n';
    }

    return output;
}

/** Constructor.
 *
 *  @param type The result type (i.e., Success, Failure, Error)
 *  @param description A description of the test performed.
 *  @param file The file where the test occured.
 *  @param line The line in the file where the test occured.
 *  @param threadId The thread that performed the test.
 *  @param date The time the result was recorded.
 */
ESTFResultCollector::ESTFResult::ESTFResult( ESTFResultType type,
    const char *description, const char *file, int line,
    ESTFThread::ThreadId threadId,
    ESTFDate date ) : _type( type ), _file( file ),
    _line( line ), _threadId( threadId ), _date( date )
{
    if ( ! description ) return;

    _description.append( description );
}

/** Destructor. */
ESTFResultCollector::ESTFResult::~ESTFResult()
{
}

/** Print the results contained in this result record to an output
 *  stream.
 *
 *  @param output The output stream.
 *  @return The output stream (for chaining).
 */
ostream &
ESTFResultCollector::ESTFResult::print( ostream &output ) const
{
    switch ( _type )
    {
        case Success:
            output << "Success,";
            break;
        case Failure:
            output << "Failure,";
            break;
        case Error:
            output << "Error,";
            break;
    }

    output << _description;

    output << ',';

    output << _file;

    output << ',';

    output << _line;

    output << ',';

    output << _threadId;

    output << ',';

    output << _date;

    return output;
}
