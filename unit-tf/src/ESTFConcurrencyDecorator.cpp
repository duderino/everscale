#ifndef ESTF_CONCURRENCY_DECORATOR_H
#include <ESTFConcurrencyDecorator.h>
#endif

#ifndef ESTF_OBJECT_PTR_H
#include <ESTFObjectPtr.h>
#endif

#ifndef ESTF_COMPONENT_THREAD_H
#include <ESTFComponentThread.h>
#endif

namespace ESTF {

ConcurrencyDecorator::ConcurrencyDecorator( ComponentPtr &component,
    int threads ) : Component(), _component( component ), _threads( threads )
{
}

ConcurrencyDecorator::~ConcurrencyDecorator()
{
}

bool
ConcurrencyDecorator::run( ResultCollector *collector )
{
    int i = 0;
    bool result = true;
    ResultCollector *collectors = 0;
    ComponentThread *threads = 0;
    ComponentPtr component;

    collectors = new ResultCollector[_threads];

    if ( ! collectors ) return false;

    threads = new ComponentThread[_threads];

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
ConcurrencyDecorator::setup()
{
    return _component->setup();
}

bool
ConcurrencyDecorator::tearDown()
{
    return _component->tearDown();
}

ComponentPtr
ConcurrencyDecorator::clone()
{
    ComponentPtr component = _component->clone();
    ConcurrencyDecoratorPtr decorator =
        new ConcurrencyDecorator( component, _threads );

    return decorator;
}

}
