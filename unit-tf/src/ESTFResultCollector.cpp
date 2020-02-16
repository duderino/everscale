#ifndef ESTF_RESULT_COLLECTOR_H
#include "ESTFResultCollector.h"
#endif

namespace ESTF {

/** Default constructor. */
ResultCollector::ResultCollector() : _results(), _successes( 0 ),
    _failures( 0 ), _errors( 0 )
{
}

/** Destructor. */
ResultCollector::~ResultCollector()
{
    _results.clear();
}

void
ResultCollector::add( ResultType type, const char *description,
    const char *file, int line )
{
    Result result( type, description, file, line,
        Thread::GetThreadId(), Date::GetSystemTime() );

    _results.insert( result );
}

/** Add a success result of a test case to a result collector.
 *
 *    @param description The result description (e.g., "x == 12").
 *    @param file The file (e.g., "my_test.cpp").
 *    @param line The line of the test (e.g., 1231).
 */
void
ResultCollector::addSuccess( const char *description, const char *file,
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
ResultCollector::addFailure( const char *description, const char *file,
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
ResultCollector::addError( const char *description, const char *file,
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
ResultCollector::merge( const ResultCollector *collector )
{
    std::multiset<Result>::iterator it = collector->_results.begin();
    std::multiset<Result>::iterator end = collector->_results.end();

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
std::ostream &
ResultCollector::print( std::ostream &output ) const
{
    /** @todo print header. */

    std::multiset<Result>::iterator it = _results.begin();
    std::multiset<Result>::iterator end = _results.end();

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
ResultCollector::Result::Result( ResultType type,
    const char *description, const char *file, int line,
    Thread::ThreadId threadId,
    Date date ) : _type( type ), _file( file ),
    _line( line ), _threadId( threadId ), _date( date )
{
    if ( ! description ) return;

    _description.append( description );
}

/** Destructor. */
ResultCollector::Result::~Result()
{
}

/** Print the results contained in this result record to an output
 *  stream.
 *
 *  @param output The output stream.
 *  @return The output stream (for chaining).
 */
std::ostream &
ResultCollector::Result::print( std::ostream &output ) const
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

}
