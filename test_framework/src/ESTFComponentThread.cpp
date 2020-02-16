/** @file ESTFComponentThread.cpp
 *  @brief A thread that runs a ESTFComponent
 *
 * Copyright (c) 2009 Yahoo! Inc.
 * The copyrights embodied in the content of this file are licensed by Yahoo! Inc.
 * under the BSD (revised) open source license.
 *
 * Derived from code that is Copyright (c) 2009 Joshua Blatt and offered under both
 * BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/).
 */

#ifndef ESTF_COMPONENT_THREAD_H
#include <ESTFComponentThread.h>
#endif

namespace ESTF {

ComponentThread::ComponentThread() : _component(), _collector( 0 ),
    _result( false )
{
}

ComponentThread::~ComponentThread()
{
}

void
ComponentThread::setComponent( ComponentPtr &component )
{
    _component = component;
}

void
ComponentThread::setCollector( ResultCollector *collector )
{
    _collector = collector;
}

bool
ComponentThread::getResult()
{
    return _result;
}

ResultCollector *
ComponentThread::getCollector()
{
    return _collector;
}

void
ComponentThread::run()
{
    _result = _component->run( _collector );
}
}
