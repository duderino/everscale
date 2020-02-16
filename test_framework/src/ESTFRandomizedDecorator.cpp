/** @file ESTFRandomizedDecorator.cpp
 *  @brief RandomizedDecorators run their decorated test case at a
 *      configurable probability.
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 */

#ifndef ESTF_RANDOMIZED_DECORATOR_H
#include "ESTFRandomizedDecorator.h"
#endif

namespace ESTF {

RandomizedDecorator::RandomizedDecorator( ComponentPtr &component,
    int prob ) : _component( component ), _rand(), _prob( prob )
{
}

RandomizedDecorator::~RandomizedDecorator()
{
}

bool
RandomizedDecorator::run( ResultCollector *collector )
{
    if ( 0 == _rand.generateRandom( 0, _prob ) )
    {
        return _component->run( collector );
    }

    return true;
}

bool
RandomizedDecorator::setup()
{
    return _component->setup();
}

bool
RandomizedDecorator::tearDown()
{
    return _component->tearDown();
}

ComponentPtr
RandomizedDecorator::clone()
{
    ComponentPtr component = _component->clone();
    RandomizedDecoratorPtr decorator =
        new RandomizedDecorator( component, _prob );

    return decorator;
}

}
