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
