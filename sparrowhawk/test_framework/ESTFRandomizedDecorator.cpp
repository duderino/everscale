/** @file ESTFRandomizedDecorator.cpp
 *  @brief ESTFRandomizedDecorators run their decorated test case at a
 *      configurable probability.
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

#ifndef ESTF_RANDOMIZED_DECORATOR_H
#include "ESTFRandomizedDecorator.h"
#endif

ESTFRandomizedDecorator::ESTFRandomizedDecorator( ESTFComponentPtr &component,
    int prob ) : _component( component ), _rand(), _prob( prob )
{
}

ESTFRandomizedDecorator::~ESTFRandomizedDecorator()
{
}

bool
ESTFRandomizedDecorator::run( ESTFResultCollector *collector )
{
    if ( 0 == _rand.generateRandom( 0, _prob ) )
    {
        return _component->run( collector );
    }

    return true;
}

bool
ESTFRandomizedDecorator::setup()
{
    return _component->setup();
}

bool
ESTFRandomizedDecorator::tearDown()
{
    return _component->tearDown();
}

ESTFComponentPtr
ESTFRandomizedDecorator::clone()
{
    ESTFComponentPtr component = _component->clone();
    ESTFRandomizedDecoratorPtr decorator =
        new ESTFRandomizedDecorator( component, _prob );

    return decorator;
}
