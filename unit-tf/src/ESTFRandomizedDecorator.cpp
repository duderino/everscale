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
