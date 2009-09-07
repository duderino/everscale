/**	@file ESFListTest.cpp
 *	@brief ESFListTest is the unit test for ESFList.
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

#ifndef ESF_LIST_TEST_H
#include <ESFListTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_ASSERT_H
#include <ESFAssert.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

#include <stdio.h>

ESFNullLock ESFListTest::_Lock;
const int ESFListTest::_Iterations = 500;
const int ESFListTest::_Records = 1000;

static const bool Debug = false;

ESFListTest::ESFListTest() :
    _records(0), _rand(), _list(ESFSystemAllocator::GetInstance(), &_Lock), _stlList() {
}

ESFListTest::~ESFListTest() {
}

bool ESFListTest::run(ESTFResultCollector *collector) {
    bool stlResult = false;
    ESFListIterator iterator;
    STLListIterator stlIterator;

    _records = new Record[_Records];

    if (!_records) {
        ESTF_ERROR( collector, "Couldn't allocate memory" );
        return false;
    }

    for ( int k = 0; k < 3; ++k )
    {
        for ( int i = 0; i < _Records; ++i )
        {
            _records[i]._value = 0;
            _records[i]._lifetime = 0;
            _records[i]._useIterator = false;
        }

        for ( int i = 0; i < _Iterations; ++i )
        {
            for ( int j = 0; j < _Records; ++j )
            {
                if ( ! _records[j]._value && 1 == _rand.generateRandom( 1, 200 ) )
                {
                    //
                    // Create and insert a new node.
                    //

                    _records[j]._value = generateValue( i, j );
                    _records[j]._lifetime = i + generateLifetime();
                    _records[j]._useIterator = ( 1 == _rand.generateRandom( 1,
                                    2 ) );

                    if ( 1 == _rand.generateRandom( 1, 2 ) )
                    {
                        _stlList.push_back( _records[j]._value );

                        ESTF_ASSERT( collector, ESF_SUCCESS ==
                                _list.pushBack( _records[j]._value ) );

                        if ( _records[j]._useIterator )
                        {
                            _records[j]._iterator = _list.getBackIterator();

                            ESTF_ASSERT( collector,
                                    ! _records[j]._iterator.isNull() );
                        }
                    }
                    else
                    {
                        _stlList.push_front( _records[j]._value );

                        ESTF_ASSERT( collector, ESF_SUCCESS ==
                                _list.pushFront( _records[j]._value ) );

                        if ( _records[j]._useIterator )
                        {
                            _records[j]._iterator = _list.getFrontIterator();

                            ESTF_ASSERT( collector,
                                    ! _records[j]._iterator.isNull() );
                        }
                    }

                    if ( Debug )
                    {
                        cerr << "Inserted: " << ( char * ) _records[j]._value
                        << " (List size: " << _list.getSize()
                        << " stl size: " << _stlList.size()
                        << ") at time " << i << endl;
                    }

                    validateList( collector );
                }

                if ( _records[j]._value && i == _records[j]._lifetime )
                {
                    //
                    //  Erase and delete an existing node.
                    //

                    if ( _records[j]._useIterator )
                    {
                        ESTF_ASSERT( collector, ESF_SUCCESS ==
                                _list.erase( &_records[j]._iterator ) );

                        ESTF_ASSERT( collector,
                                _records[j]._iterator.isNull() );
                    }
                    else
                    {
                        iterator = findIterator( _records[j]._value );

                        ESTF_ASSERT( collector, ! iterator.isNull() );

                        ESTF_ASSERT( collector,
                                ESF_SUCCESS == _list.erase( &iterator ) );

                        ESTF_ASSERT( collector, iterator.isNull() );
                    }

                    if ( Debug )
                    {
                        cerr << "value[" << j << "]: "
                        << _records[j]._value
                        << endl;
                    }

                    stlResult = findSTLIterator( _records[j]._value, &stlIterator );

                    ESF_ASSERT( stlResult );

                    _stlList.erase( stlIterator );

                    if ( Debug )
                    {
                        cerr << "Deleted: " << ( char * ) _records[j]._value
                        << " (list size: " << _list.getSize() << " stl size: "
                        << _stlList.size() << ") at time " << i << endl;
                    }

                    delete[] _records[j]._value;
                    _records[j]._value = 0;

                    validateList( collector );
                }
            }
        }

        //
        //  Cleanup
        //

        ESFListIterator temp;
        char *value = 0;

        for ( value = ( char * ) _list.getFront();
                ! _list.isEmpty();
                value = ( char * ) _list.getFront()
        )
        {
            ESTF_ASSERT( collector, ESF_SUCCESS == _list.popFront() );

            stlResult = findSTLIterator( value, &stlIterator );

            ESF_ASSERT( stlResult );

            _stlList.erase( stlIterator );

            if ( Debug )
            {
                cerr << "Deleted: " << value
                << " (List size: " << _list.getSize() << " stl size: "
                << _stlList.size() << ") at cleanup stage" << endl;
            }

            delete[] value;

            validateList( collector );
        }
    }

    delete[] _records;
    _records = 0;

    return true;
}

ESFListIterator ESFListTest::findIterator(void *value) {
    ESFListIterator it;

    for (it = _list.getFrontIterator(); it.hasNext(); it = it.getNext()) {
        if (0 == strcmp((char *) value, (char *) it.getValue())) {
            return it;
        }
    }

    return it;
}

bool ESFListTest::findSTLIterator(void *value, STLListIterator *it) {
    for (*it = _stlList.begin(); *it != _stlList.end(); ++(*it)) {
        if (0 == strcmp((char *) value, **it)) {
            return true;
        }
    }

    return false;
}

void ESFListTest::validateList(ESTFResultCollector *collector) {
    ESFListIterator iterator;
    STLListIterator stlIterator;
    char *value = 0;
    ESFUInt32 counter = 0;

    ESTF_ASSERT( collector, _list.getSize() == _stlList.size() );

    for (stlIterator = _stlList.begin(); stlIterator != _stlList.end(); ++stlIterator) {
        iterator = findIterator(*stlIterator);

        ESTF_ASSERT( collector, ! iterator.isNull() );
    }

    //
    //  Forward iterate through both lists and make sure that their records
    //  are in the same order.
    //

    for (stlIterator = _stlList.begin(), iterator = _list.getFrontIterator(); !iterator.isNull(); ++stlIterator, iterator
            = iterator.getNext(), ++counter) {
        value = (char *) iterator.getValue();

        ESTF_ASSERT( collector, 0 == strcmp( value, *stlIterator ) );
    }

    ESTF_ASSERT( collector, ! iterator.hasNext() );
    ESTF_ASSERT( collector, counter == _list.getSize() );

    //
    //  Reverse iterate through both lists and make sure that their records
    //  are in the same order.
    //
    counter = 0;
    stlIterator = _stlList.end();

    for (--stlIterator, iterator = _list.getBackIterator(); !iterator.isNull(); --stlIterator, iterator
            = iterator.getPrevious(), ++counter) {
        value = (char *) iterator.getValue();

        ESTF_ASSERT( collector, 0 == strcmp( value, *stlIterator ) );
    }

    ESTF_ASSERT( collector, ! iterator.hasPrevious() );
    ESTF_ASSERT( collector, counter == _list.getSize() );
}

bool ESFListTest::setup() {
    return true;
}

bool ESFListTest::tearDown() {
    return true;
}

ESTFComponentPtr ESFListTest::clone() {
    ESTFComponentPtr component(new ESFListTest());

    return component;
}

char *
ESFListTest::generateValue(int i, int j) {
    char *value = new char[22];

    if (!value)
        return 0;

    sprintf(value, "%d@%d", i, j);

    return value;
}

int ESFListTest::generateLifetime() {
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
