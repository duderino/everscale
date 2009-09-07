/**    @file ESTFConcurrencyComposite.cpp
 *    @brief ESTFConcurrencyComposites contain many ESTFComponents and run them
 *      concurrently.
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

#ifndef ESTF_CONCURRENCY_COMPOSITE_H
#include <ESTFConcurrencyComposite.h>
#endif

#ifndef ESTF_COMPONENT_THREAD_H
#include <ESTFComponentThread.h>
#endif

ESTFConcurrencyComposite::ESTFConcurrencyComposite() : ESTFComposite()
{
}

ESTFConcurrencyComposite::~ESTFConcurrencyComposite()
{
}

bool
ESTFConcurrencyComposite::run( ESTFResultCollector *collector )
{
    int numThreads = _children.size();
    bool result = true;
    ESTFResultCollector *collectors = 0;
    ESTFComponentThread *threads = 0;
    std::list<ESTFComponentPtr>::iterator it = _children.begin();
    std::list<ESTFComponentPtr>::iterator end = _children.end();

    collectors = new ESTFResultCollector[numThreads];

    if ( ! collectors )
    {
        return false;
    }

    threads = new ESTFComponentThread[numThreads];

    if ( ! threads )
    {
        delete[] collectors;
        return false;
    }

    for ( int i = 0; i < numThreads && it != end; ++i, ++it )
    {
        threads[i].setComponent( *it );
        threads[i].setCollector( &collectors[i] );
    }

    for ( int i = 0; i < numThreads; ++i )
    {
        threads[i].spawn();
    }

    for ( int i = 0; i < numThreads; ++i )
    {
        threads[i].join();
    }

    for ( int i = 0; i < numThreads; ++i )
    {
        if ( true == threads[i].getResult() )
        {
            collector->merge( threads[i].getCollector() );
        }
        else
        {
            result = false;
        }
    }

    delete[] collectors;
    delete[] threads;

    return result;
}

ESTFComponentPtr
ESTFConcurrencyComposite::clone()
{
    //
    //  could replace the new ESTFConcurrencyComposite with a template
    //  method and just inherit the rest of this method...
    //
    std::list<ESTFComponentPtr>::iterator it = _children.begin();
    std::list<ESTFComponentPtr>::iterator end = _children.end();
    ESTFCompositePtr composite = new ESTFConcurrencyComposite();
    ESTFComponentPtr object;

    if ( composite.isNull() )
    {
        ESTF_NATIVE_ASSERT( 0 == "alloc failed" );
        return composite;
    }

    for ( ; it != end; ++it )
    {
        object = (*it)->clone();

        composite->add( object );
    }

    return composite;
}
