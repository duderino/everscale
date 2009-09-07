/**    @file ESTFConcurrencyDecorator.cpp
 *  @brief ESTFConcurrencyDecorators run their decorated test case in multiple
 *      threads.
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

#ifndef ESTF_CONCURRENCY_DECORATOR_H
#include <ESTFConcurrencyDecorator.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_THREAD_H
#include <ESTFComponentThread.h>
#endif

ESTFConcurrencyDecorator::ESTFConcurrencyDecorator( ESTFComponentPtr &component,
    int threads ) : ESTFComponent(), _component( component ), _threads( threads )
{
}

ESTFConcurrencyDecorator::~ESTFConcurrencyDecorator()
{
}

bool
ESTFConcurrencyDecorator::run( ESTFResultCollector *collector )
{
    int i = 0;
    bool result = true;
    ESTFResultCollector *collectors = 0;
    ESTFComponentThread *threads = 0;
    ESTFComponentPtr component;

    collectors = new ESTFResultCollector[_threads];

    if ( ! collectors ) return false;

    threads = new ESTFComponentThread[_threads];

    if ( ! threads )
    {
        delete[] collectors;
        return false;
    }

    for ( i = 0; i < _threads; ++i )
    {
        component = _component->clone();

        threads[i].setComponent( component );
        threads[i].setCollector( &collectors[i] );
    }

    for ( i = 0; i < _threads; ++i )
    {
        threads[i].spawn();
    }

    for ( i = 0; i < _threads; ++i )
    {
        threads[i].join();
    }

    for ( i = 0; i < _threads; ++i )
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

bool
ESTFConcurrencyDecorator::setup()
{
    return _component->setup();
}

bool
ESTFConcurrencyDecorator::tearDown()
{
    return _component->tearDown();
}

ESTFComponentPtr
ESTFConcurrencyDecorator::clone()
{
    ESTFComponentPtr component = _component->clone();
    ESTFConcurrencyDecoratorPtr decorator =
        new ESTFConcurrencyDecorator( component, _threads );

    return decorator;
}
