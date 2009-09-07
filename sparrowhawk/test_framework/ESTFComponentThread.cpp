/** @file ESTFComponentThread.cpp
 *  @brief A thread that runs a ESTFComponent
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

#ifndef ESTF_COMPONENT_THREAD_H
#include <ESTFComponentThread.h>
#endif

ESTFComponentThread::ESTFComponentThread() : _component(), _collector( 0 ),
    _result( false )
{
}

ESTFComponentThread::~ESTFComponentThread()
{
}

void
ESTFComponentThread::setComponent( ESTFComponentPtr &component )
{
    _component = component;
}

void
ESTFComponentThread::setCollector( ESTFResultCollector *collector )
{
    _collector = collector;
}

bool
ESTFComponentThread::getResult()
{
    return _result;
}

ESTFResultCollector *
ESTFComponentThread::getCollector()
{
    return _collector;
}

void
ESTFComponentThread::run()
{
    _result = _component->run( _collector );
}
