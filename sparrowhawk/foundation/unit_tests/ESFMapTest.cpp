/**	@file ESFMapTest.cpp
 *	@brief ESFMapTest is the unit test for ESFMap.
 *
 *  Copyright 2005 Joshua Blatt, Yahoo! Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  $Author: blattj $
 *  $Date: 2009/05/25 21:51:14 $
 *  $Name:  $
 *  $Revision: 1.3 $
 */

#ifndef ESF_MAP_TEST_H
#include <ESFMapTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_WRITE_SCOPE_LOCK_H
#include <ESFWriteScopeLock.h>
#endif

#include <stdio.h>

ESFMapTest::StringComparator ESFMapTest::_Comparator;
ESFNullLock ESFMapTest::_Lock;
const int ESFMapTest::_Iterations = 2000;
const int ESFMapTest::_Records = 1000;

ESFMutex StlLock;

static const bool Debug = false;

ESFMapTest::ESFMapTest(bool isUnique) :
    _isUnique(isUnique), _records(0), _rand(), _map(isUnique, &_Comparator, ESFSystemAllocator::GetInstance(), &_Lock),
            _stlMultiMap(), _stlMap() {
}

ESFMapTest::~ESFMapTest() {
}

bool ESFMapTest::run(ESTFResultCollector *collector) {
    ESFError error;
    bool stlResult = false;
    ESFMapIterator iterator;

    _records = new Record[_Records];

    if (!_records) {
        ESTF_ERROR( collector, "Couldn't allocate memory" );
        return false;
    }

    for ( int k = 0; k < 3; ++k )
    {
        for ( int i = 0; i < _Iterations; ++i )
        {
            for ( int j = 0; j < _Records; ++j )
            {
                if ( ! _records[j]._key && 1 == _rand.generateRandom( 1, 200 ) )
                {
                    //
                    // Create and insert a new node.
                    //

                    _records[j]._key = generateKey();
                    _records[j]._value = generateValue( i, j );
                    _records[j]._lifetime = i + generateLifetime();
                    _records[j]._useIterator = ( 1 ==
                            _rand.generateRandom( 1, 2 ) );

                    if ( _isUnique )
                    {
                        //ESFWriteScopeLock scopeLock( StlLock );

                        stlResult = _stlMap.insert( std::pair<const char *, char *>(
                                        ( const char * ) _records[j]._key,
                                        ( char * ) _records[j]._value ) ).second;
                    }
                    else
                    {
                        //ESFWriteScopeLock scopeLock( StlLock );

                        _stlMultiMap.insert( std::pair<const char *, char *>(
                                        ( const char * ) _records[j]._key,
                                        ( char * ) _records[j]._value ) );

                        stlResult = true;
                    }

                    if ( _records[j]._useIterator )
                    {
                        error = _map.insert( _records[j]._key,
                                _records[j]._value,
                                &_records[j]._iterator );

                        if ( ESF_SUCCESS == error )
                        {
                            ESTF_ASSERT( collector,
                                    ! _records[j]._iterator.isNull() );
                        }
                        else
                        {
                            ESTF_ASSERT( collector,
                                    _records[j]._iterator.isNull() );
                        }
                    }
                    else
                    {
                        error = _map.insert( _records[j]._key,
                                _records[j]._value );
                    }

                    //
                    //  Map inserts are allowed to fail if the STL insert
                    //  failed.
                    //
                    ESTF_ASSERT( collector,
                            stlResult == ( ESF_SUCCESS == error ) );

                    if ( ESF_SUCCESS != error )
                    {
                        if ( Debug )
                        {
                            cerr << "Failed to insert: "
                            << ( char * ) _records[j]._key
                            << " at time " << i << endl;
                        }

                        delete[] ( char * ) _records[j]._key;
                        _records[j]._key = 0;
                        delete[] ( char * ) _records[j]._value;
                        _records[j]._value = 0;
                    }
                    else
                    {
                        if ( Debug )
                        {
                            //ESFWriteScopeLock scopeLock( StlLock );

                            cerr << "Inserted: " << ( char * ) _records[j]._key
                            << " (Map size: " << _map.getSize()
                            << " stl size: " << _stlMap.size()
                            << ") at time " << i << endl;
                        }
                    }

                    validateTree( collector );
                }

                if ( _records[j]._key && i == _records[j]._lifetime )
                {
                    //
                    //  Erase and delete an existing node.
                    //

                    if ( _records[j]._useIterator )
                    {
                        error = _map.erase( &_records[j]._iterator );

                        ESTF_ASSERT( collector, ESF_SUCCESS == error );

                        if ( _isUnique )
                        {
                            //ESFWriteScopeLock scopeLock( StlLock );

                            _stlMap.erase( ( const char * ) _records[j]._key );
                        }
                        else
                        {
                            assert( 0 == "erase from multimap" );
                        }
                    }
                    else
                    {
                        if ( _isUnique )
                        {
                            error = _map.erase( _records[j]._key );

                            ESTF_ASSERT( collector, ESF_SUCCESS == error );

                            void *dummy = 0;

                            error = _map.find( _records[j]._key,
                                    &dummy );

                            ESTF_ASSERT( collector, ESF_CANNOT_FIND == error );

                            //ESFWriteScopeLock scopeLock( StlLock );

                            _stlMap.erase( ( const char * ) _records[j]._key );
                        }
                        else
                        {
                            error = _map.find( _records[j]._key,
                                    &iterator );

                            ESTF_ASSERT( collector, ! iterator.isNull() );
                            ESTF_ASSERT( collector, ESF_SUCCESS == error );

                            //
                            //  The way we generated the values, even if we
                            //  have multiple elements with the same key, their
                            //  values are guaranteed to be different.
                            //

                            if ( iterator.getValue() == _records[j]._value )
                            {
                                error = _map.erase( &iterator );

                                ESTF_ASSERT( collector, ESF_SUCCESS == error );
                            }
                            else
                            {
                                ESFMapIterator next;

                                error = ESF_CANNOT_FIND;

                                while ( iterator.hasNext() )
                                {
                                    next = iterator.getNext();

                                    if ( _Comparator.compare( next.getKey(),
                                                    _records[j]._key ) )
                                    {
                                        break;
                                    }

                                    if ( next.getValue() == _records[j]._value )
                                    {
                                        error = _map.erase( &next );

                                        break;
                                    }
                                }

                                ESTF_ASSERT( collector, ESF_SUCCESS == error );
                            }
                        }
                    }

                    if ( Debug )
                    {
                        //ESFWriteScopeLock scopeLock( StlLock );

                        cerr << "Deleted: " << ( char * ) _records[j]._key
                        << " (Map size: " << _map.getSize() << " stl size: "
                        << _stlMap.size() << ") at time " << i << endl;
                    }

                    delete[] ( char * ) _records[j]._key;
                    _records[j]._key = 0;
                    delete[] ( char * ) _records[j]._value;
                    _records[j]._value = 0;

                    validateTree( collector );
                }
            }
        }

        //
        //  Cleanup
        //

        ESFMapIterator temp;
        char *key = 0;
        char *value = 0;

        for ( iterator = _map.getMinimumIterator();
                ! iterator.isNull();
                iterator = temp
        )
        {
            key = ( char * ) iterator.getKey();
            value = ( char * ) iterator.getValue();

            if ( _isUnique )
            {
                //ESFWriteScopeLock scopeLock( StlLock );

                _stlMap.erase( key );
            }
            else
            {
                assert( 0 == "erase from multimap" );
            }

            temp = iterator.getNext();

            error = _map.erase( &iterator );

            ESTF_ASSERT( collector, ESF_SUCCESS == error );

            if ( Debug )
            {
                //ESFWriteScopeLock scopeLock( StlLock );

                cerr << "Deleted: " << key
                << " (Map size: " << _map.getSize() << " stl size: "
                << _stlMap.size() << ") at cleanup stage" << endl;
            }

            delete[] ( char * ) key;
            key = 0;
            delete[] ( char * ) value;
            value = 0;

            validateTree( collector );
        }
    }

    delete[] _records;
    _records = 0;

    return true;
}

void ESFMapTest::validateTree(ESTFResultCollector *collector) {
    ESFMapIterator iterator;
    ESFError error;
    void *value = 0;

    //
    //  Run through the stl map or multimap and verify that we can find
    //  every record in the map in the tree.
    //

    if (_isUnique) {
        //ESFWriteScopeLock scopeLock( StlLock );

        std::map<const char *, char *, STLStringComparator>::iterator it;

        for (it = _stlMap.begin(); it != _stlMap.end(); ++it) {
            error = _map.find(it->first, &value);

            if (Debug && !value) {
                cerr << "Couldn't find: " << (char *) it->first << endl;
            }

            ESTF_ASSERT( collector, ESF_SUCCESS == error );
            ESTF_ASSERT( collector, value );

            ESTF_ASSERT( collector,
                    0 == _Comparator.compare( value,
                            it->second ) );
        }
    } else {
        //ESFWriteScopeLock scopeLock( StlLock );

        std::multimap<const char*, char *, STLStringComparator>::iterator it;

        for (it = _stlMultiMap.begin(); it != _stlMultiMap.end(); ++it) {
            error = _map.find(it->first, &iterator);

            ESTF_ASSERT( collector, ! iterator.isNull() );
            ESTF_ASSERT( collector, ESF_SUCCESS == error );

            if (iterator.isNull())
                continue;

            error = ESF_CANNOT_FIND;

            while (true) {
                if (0 != _Comparator.compare(iterator.getKey(), it->first)) {
                    break;
                }

                if (0 == _Comparator.compare(iterator.getValue(), it->second)) {
                    error = ESF_SUCCESS;
                    break;
                }
            }

            ESTF_ASSERT( collector, ESF_SUCCESS == error );
        }
    }

    //
    //  Make sure that the tree is balanced.
    //

#ifdef DEBUG
    ESTF_ASSERT( collector, true == _map.isBalanced() );
#endif

    //
    //  Make sure our sizes are right.
    //

    if (_isUnique) {
        //ESFWriteScopeLock scopeLock( StlLock );

        ESTF_ASSERT( collector, _map.getSize() == _stlMap.size() );
    } else {
        //ESFWriteScopeLock scopeLock( StlLock );

        ESTF_ASSERT( collector, _map.getSize() == _stlMultiMap.size() );
    }

    if (2 > _map.getSize())
        return;

    //
    //  Iterate through the map in forward order and make sure the keys
    //  are in ascending order.
    //

    ESFMapIterator next;
    iterator = _map.getMinimumIterator();
    ESFUInt32 i = 1;
    int comparison = 0;

    while (true) {
        next = iterator.getNext();

        if (next.isNull()) {
            break;
        }

        comparison = _Comparator.compare(next.getKey(), iterator.getKey());

        ESTF_ASSERT( collector, 0 <= comparison );

        iterator = next;
        ++i;
    }

    ESTF_ASSERT( collector, i == _map.getSize() );

    //
    //  Iterate through the map in reverse order and make sure the keys
    //  are in descending order.
    //

    ESFMapIterator prev;
    iterator = _map.getMaximumIterator();
    i = 1;
    comparison = 0;

    while (true) {
        prev = iterator.getPrevious();

        if (prev.isNull()) {
            break;
        }

        comparison = _Comparator.compare(prev.getKey(), iterator.getKey());

        ESTF_ASSERT( collector, 0 >= comparison );

        iterator = prev;
        ++i;
    }

    ESTF_ASSERT( collector, i == _map.getSize() );
}

bool ESFMapTest::setup() {
    return true;
}

bool ESFMapTest::tearDown() {
    return true;
}

ESTFComponentPtr ESFMapTest::clone() {
    ESTFComponentPtr component(new ESFMapTest(_isUnique));

    return component;
}

char *
ESFMapTest::generateKey() {
    char *key = new char[4];

    if (!key)
        return 0;

    for (int i = 0; i < 3; ++i) {
        key[i] = _rand.generateRandom(65, 90);
    }

    key[3] = '\0';

    return key;
}

char *
ESFMapTest::generateValue(int i, int j) {
    char *value = new char[22];

    if (!value)
        return 0;

    sprintf(value, "%d@%d", i, j);

    return value;
}

int ESFMapTest::generateLifetime() {
    int uniformDeviate = _rand.generateRandom(1, 3);

    switch (uniformDeviate) {
    case 1:
        return 1;

    case 2:
        return _rand.generateRandom(1, 10);

    case 3:
        return _rand.generateRandom(1, 100);
    }

    return 10;
}
