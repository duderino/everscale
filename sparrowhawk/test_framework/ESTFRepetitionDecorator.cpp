/** @file ESTFRepetitionDecorator.cpp
 *  @brief ESTFRepetitionDecorators run their decorated test case for a
 *      configurable number of repetitions.
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

#ifndef ESTF_REPETITION_DECORATOR_H
#include "ESTFRepetitionDecorator.h"
#endif

ESTFRepetitionDecorator::ESTFRepetitionDecorator( ESTFComponentPtr &component,
    int repetitions ) : ESTFComponent(), _component( component ),
    _repetitions( repetitions )
{
}

ESTFRepetitionDecorator::~ESTFRepetitionDecorator()
{
}

bool
ESTFRepetitionDecorator::run( ESTFResultCollector *collector )
{
    for ( int i = 0; i < _repetitions; ++i )
    {
        if ( false == _component->run( collector ) )
        {
            return false;
        }
    }

    return true;
}

bool
ESTFRepetitionDecorator::setup()
{
    return _component->setup();
}

bool
ESTFRepetitionDecorator::tearDown()
{
    return _component->tearDown();
}

ESTFComponentPtr
ESTFRepetitionDecorator::clone()
{
    ESTFComponentPtr component = _component->clone();
    ESTFRepetitionDecoratorPtr decorator =
        new ESTFRepetitionDecorator( component, _repetitions );

    return decorator;
}
