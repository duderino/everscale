/**    @file ESTFComposite.cpp
 *  @brief ESTFComposites contain many ESTFComponents and run them serially.
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

#ifndef ESTF_COMPOSITE_H
#include "ESTFComposite.h"
#endif

ESTFComposite::ESTFComposite() : ESTFComponent(), _children()
{
}

ESTFComposite::~ESTFComposite()
{
    _children.clear();
}

bool ESTFComposite::run( ESTFResultCollector *collector )
{
    std::list<ESTFComponentPtr>::iterator it = _children.begin();
    std::list<ESTFComponentPtr>::iterator end = _children.end();

    for ( ; it != end; ++it )
    {
        if ( true != (*it)->run( collector ) )
        {
            return false;
        }
    }

    return true;
}

bool ESTFComposite::setup()
{
    std::list<ESTFComponentPtr>::iterator it = _children.begin();
    std::list<ESTFComponentPtr>::iterator end = _children.end();

    for ( ; it != end; ++it )
    {
        if ( true != (*it)->setup() )
        {
            return false;
        }
    }

    return true;
}

bool ESTFComposite::tearDown()
{
    std::list<ESTFComponentPtr>::iterator it = _children.begin();
    std::list<ESTFComponentPtr>::iterator end = _children.end();

    for ( ; it != end; ++it )
    {
        if ( true != (*it)->tearDown() )
        {
            return false;
        }
    }

    return true;
}

ESTFComponentPtr ESTFComposite::clone()
{
    std::list<ESTFComponentPtr>::iterator it = _children.begin();
    std::list<ESTFComponentPtr>::iterator end = _children.end();
    ESTFCompositePtr composite = new ESTFComposite();
    ESTFComponentPtr object;

    if ( composite.isNull() )
    {
        ESTF_NATIVE_ASSERT( 0 == "alloc failed" );
        return composite;
    }

    for ( ; it != end; ++it )
    {
        object = (*it)->clone();

        composite->add( object );
    }

    return composite;
}

void ESTFComposite::add( ESTFComponentPtr &component )
{
    _children.push_back( component );
}

void ESTFComposite::remove( const ESTFComponentPtr &component )
{
    std::list<ESTFComponentPtr>::iterator it = _children.begin();
    std::list<ESTFComponentPtr>::iterator end = _children.end();

    for ( ; it != end; ++it )
    {
        if ( (*it) == component )
        {
            _children.erase( it );
            return;
        }
    }
}

void
ESTFComposite::clear()
{
    _children.clear();
}

int ESTFComposite::size()
{
    return _children.size();
}
