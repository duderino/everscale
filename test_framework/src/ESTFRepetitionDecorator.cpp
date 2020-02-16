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
 */

#ifndef ESTF_REPETITION_DECORATOR_H
#include "ESTFRepetitionDecorator.h"
#endif

namespace ESTF {

RepetitionDecorator::RepetitionDecorator( ComponentPtr &component,
    int repetitions ) : Component(), _component( component ),
    _repetitions( repetitions )
{
}

RepetitionDecorator::~RepetitionDecorator()
{
}

bool
RepetitionDecorator::run( ResultCollector *collector )
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
RepetitionDecorator::setup()
{
    return _component->setup();
}

bool
RepetitionDecorator::tearDown()
{
    return _component->tearDown();
}

ComponentPtr
RepetitionDecorator::clone()
{
    ComponentPtr component = _component->clone();
    RepetitionDecoratorPtr decorator =
        new RepetitionDecorator( component, _repetitions );

    return decorator;
}

}
