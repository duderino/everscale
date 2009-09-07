/**	@file ESFBuddyAllocatorTest.cpp
 *	@brief ESFBuddyAllocatorTest is the unit test for ESFBuddyAllocator.
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

#ifndef ESF_BUDDY_ALLOCATOR_TEST_H
#include <ESFBuddyAllocatorTest.h>
#endif

#ifndef ESTF_ASSERT_H
#include <ESTFAssert.h>
#endif

#ifndef ESF_SYSTEM_ALLOCATOR_H
#include <ESFSystemAllocator.h>
#endif

static const int Allocations = 160;
static const bool Debug = false;
static const int Iterations = 2000;

ESFBuddyAllocatorTest::ESFBuddyAllocatorTest() :
    _rand(), _allocator(17, ESFSystemAllocator::GetInstance()) {
}

ESFBuddyAllocatorTest::~ESFBuddyAllocatorTest() {
}

bool ESFBuddyAllocatorTest::run(ESTFResultCollector *collector) {
    //
    //  This test case follows Knuth's test case for the Buddy System ( and
    //  other allocators ) in The Art of Computer Programming, Volume 1
    //  Fundamental Algorithms Third Edition, p. 146.
    //

    ESFError error;
    int bytesAllocated = 0;
    char buffer[256];

    error = _allocator.initialize();

    if (ESF_SUCCESS != error) {
        ESFDescribeError(error, buffer, sizeof(buffer));

        ESTF_FAILURE( collector, buffer );
        ESTF_ERROR( collector, "Failed to initialize allocator" );

        return false;
    }

    struct allocation
    {
        int size;
        int lifetime;
        void *data;
    }allocations[Allocations];

    memset( allocations, 0, sizeof( allocations ) );

    for ( int i = 0; i < Iterations; ++i )
    {
        for ( int j = 0; j < Allocations; ++j )
        {
            if ( 0 < allocations[j].size && allocations[j].lifetime == i )
            {
                if ( Debug )
                {
                    cerr << "Freeing block of size " << allocations[j].size
                    << " at time " << i << endl;
                }

                char *data = ( char * ) allocations[j].data;

                // Make sure no one else overwrote this block.
                for ( int k = 0; k < allocations[j].size; ++k )
                {
                    ESTF_ASSERT( collector, ( i % 127 ) == data[k] );
                }

                error = _allocator.deallocate( allocations[j].data );
                allocations[j].data = 0;

                if ( ESF_SUCCESS != error )
                {
                    ESFDescribeError( error, buffer, sizeof( buffer ) );
                    ESTF_FAILURE( collector, buffer );
                }

                ESTF_ASSERT( collector, ESF_SUCCESS == error );
                ESTF_ASSERT( collector, ! allocations[j].data );

                bytesAllocated -= allocations[j].size;

                allocations[j].size = 0;
                allocations[j].lifetime = 0;
                allocations[j].data = 0;
            }

            if ( 0 == allocations[j].size && 1 == _rand.generateRandom( 1, 4 ) )
            {
                ESTF_ASSERT( collector, 0 == allocations[j].lifetime );
                ESTF_ASSERT( collector, 0 == allocations[j].data );

                allocations[j].size = generateAllocSize();
                allocations[j].lifetime = i + generateAllocLifetime();

                if ( Debug )
                {
                    cerr << "Allocating block of size " << allocations[j].size
                    << " at time " << i << " with lifetime: "
                    << allocations[j].lifetime << endl;
                }

                error = _allocator.allocate( &allocations[j].data,
                        allocations[j].size );

                //
                //  We allow failures after half of the allocators memory
                //  has been allocated.
                //
                if ( ( bytesAllocated < ( 1 << 16 ) ) ||
                        allocations[j].data )
                {
                    if ( ESF_SUCCESS != error )
                    {
                        ESFDescribeError( error, buffer, sizeof( buffer ) );
                        ESTF_FAILURE( collector, buffer );
                    }

                    ESTF_ASSERT( collector, ESF_SUCCESS == error );
                    ESTF_ASSERT( collector, allocations[j].data );
                }
                else
                {
                    if ( ESF_OUT_OF_MEMORY != error )
                    {
                        ESFDescribeError( error, buffer, sizeof( buffer ) );
                        ESTF_FAILURE( collector, buffer );
                    }

                    ESTF_ASSERT( collector, ESF_OUT_OF_MEMORY == error );
                }

                if ( ! allocations[j].data )
                {
                    if ( Debug )
                    {
                        cerr << "Failed to allocate block of size "
                        << allocations[j].size << " at time " << i
                        << " with lifetime: "
                        << allocations[j].lifetime << endl;
                    }

                    allocations[j].lifetime = 0;
                    allocations[j].size = 0;
                }
                else
                {
                    //
                    // We will check this value when we free the block to
                    // make sure no one overwrote it.
                    //
                    memset( allocations[j].data,
                            allocations[j].lifetime % 127,
                            allocations[j].size );

                    bytesAllocated += allocations[j].size;
                }

                if ( Debug )
                {
                    cerr << bytesAllocated << " out of " << ( 1 << 17 )
                    << " bytes allocated\n";
                }
            }

            if ( Debug )
            {
                int size =0;

                for ( int k = 0; k < Allocations; ++k )
                {
                    size += allocations[k].size;
                }

                ESTF_ASSERT( collector, size == bytesAllocated );
            }
        }
    }

    // Simulation mostly done, return everything to the allocator.

    for ( int i = 0; i < Allocations; ++i )
    {
        if ( 0 < allocations[i].size )
        {
            if ( Debug )
            {
                cerr << "Freeing block of size " << allocations[i].size
                << " at cleanup stage\n";
            }

            error = _allocator.deallocate( allocations[i].data );
            allocations[i].data = 0;

            if ( ESF_SUCCESS != error )
            {
                ESFDescribeError( error, buffer, sizeof( buffer ) );
                ESTF_FAILURE( collector, buffer );
            }

            ESTF_ASSERT( collector, ESF_SUCCESS == error );
            ESTF_ASSERT( collector, ! allocations[i].data );

            allocations[i].size = 0;
            allocations[i].lifetime = 0;
            allocations[i].data = 0;
        }
    }

    //
    // The buddy allocator should have coalesced everything back into one
    // big block, try a really big allocation.
    //

    error = _allocator.allocate( &allocations[0].data, 1 << 16 );

    if ( ESF_SUCCESS != error )
    {
        ESFDescribeError( error, buffer, sizeof( buffer ) );
        ESTF_FAILURE( collector, buffer );
    }

    ESTF_ASSERT( collector, ESF_SUCCESS == error );
    ESTF_ASSERT( collector, allocations[0].data );

    if ( allocations[0].data )
    {
        memset( allocations[0].data, 'B', 1 << 16 );

        error = _allocator.destroy();

        ESTF_ASSERT( collector, ESF_IN_USE == error );

        if ( Debug )
        {
            cerr << "Allocated big block of size " << ( 1 << 16 ) << endl;
        }

        error = _allocator.deallocate( allocations[0].data );
        allocations[0].data = 0;

        if ( ESF_SUCCESS != error )
        {
            ESFDescribeError( error, buffer, sizeof( buffer ) );
            ESTF_FAILURE( collector, buffer );
        }

        ESTF_ASSERT( collector, ESF_SUCCESS == error );
        ESTF_ASSERT( collector, ! allocations[0].data );
    }
    else
    {
        cerr << "Failed to alloc big block of size " << ( 1 << 16 ) << endl;
    }

    error = _allocator.destroy();

    if ( ESF_SUCCESS != error )
    {
        ESFDescribeError( error, buffer, sizeof( buffer ) );
        ESTF_FAILURE( collector, buffer );
        ESTF_ERROR( collector, "Failed to destroy allocator" );
        return false;
    }

    ESTF_ASSERT( collector, ESF_SUCCESS == error );

    return true;
}

bool ESFBuddyAllocatorTest::setup() {
    return true;
}

bool ESFBuddyAllocatorTest::tearDown() {
    return true;
}

ESTFComponentPtr ESFBuddyAllocatorTest::clone() {
    ESTFComponentPtr component(new ESFBuddyAllocatorTest());

    return component;
}

int ESFBuddyAllocatorTest::generateAllocSize() {
    static const int array1[32] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 4, 4,
            16, 16, 32, 32 };

    static const int array2[22] = { 10, 12, 14, 16, 18, 20, 30, 40, 50, 60, 70, 80, 90, 100, 150, 200, 250, 500, 1000,
            2000, 3000, 4000 };

    int uniformDeviate = _rand.generateRandom(1, 3);

    switch (uniformDeviate) {
    case 1:
        return _rand.generateRandom(100, 2000);

    case 2:
        return array1[_rand.generateRandom(0, 31)];

    case 3:
        return array2[_rand.generateRandom(0, 21)];
    }

    return 10;
}

int ESFBuddyAllocatorTest::generateAllocLifetime() {
    int uniformDeviate = _rand.generateRandom(1, 3);

    switch (uniformDeviate) {
    case 1:
        return _rand.generateRandom(1, 10);

    case 2:
        return _rand.generateRandom(1, 100);

    case 3:
        return _rand.generateRandom(1, 1000);
    }

    return 10;
}
