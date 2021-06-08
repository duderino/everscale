Software Copyright License Agreement (BSD License)

The copyrights to the software code for Everscale and accompanying code are licensed under the following terms:

Copyright (c) 2020, Joshua Blatt.  All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* Neither the name of the organization nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL JOSHUA BLATT BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This project also uses code and concepts from the following sources:

1) Duderino.  BSD License.

Duderino is visible as commit b496c84aa3de9d3e8bb0588dbe716a1db98f9e54 in this repo and is Copyright (c) 2009 Yahoo! Inc. and offered under BSD license (https://github.com/duderino/duderino/).
Code comprising the Sparrowhawk modules included with Duderino is Copyright (c) 2009 Joshua Blatt and offered under both BSD and Apache 2.0 licenses (http://sourceforge.net/projects/sparrowhawk/)

2) CMake.  BSD License.

CMake is required to build the project and only used as a build-time (not redistributed) dependency.  Copyright 2000-2001 Kitware, Inc. and Contributors and offered under BSD 3-Clause license (https://gitlab.kitware.com/cmake/cmake/raw/master/Copyright.txt).

3) Google Test.  BSD License.

Google Test is dynamicaly fetched from https://github.com/google/googletest and is used as a build-time (not redistributed) dependency.  Copyright 2008, Google Inc. and offered under BSD 3-Clause license (https://github.com/google/googletest/commit/11841175d8023203898e4168c49e80590a19795c).

4) BoringSSL.  Multiple Licenses (OpenSSL, ISC, MIT, and more)

BoringSSL is dynamically fetched from https://boringssl.googlesource.com/boringssl and is used as a runtime (statically linked and redistributed) dependency.  See https://boringssl.googlesource.com/boringssl/+/refs/tags/fips-20190808/LICENSE for its license details.

5) gRPC.  Apache 2 top-level license, with many transitive dependencies.

gRPC is dynamically fetched from https://github.com/grpc/grpc and is used as a runtime (statically linked and redistributed) dependency.  See https://github.com/grpc/grpc/blob/v1.35.0/LICENSE for its license details, including gRPC dependencies.

6) gRPC Example Code.  Apache 2 License.

Example code from https://github.com/grpc/grpc/blob/master/examples/cpp/helloworld/ was used and is Copyright 2018 gRPC authors and offered under Apache 2 license (http://www.apache.org/licenses/LICENSE-2.0).

7) yajl JSON SAX parser.  ISC License

yajl is used for parsing JSON.  It is fetched at buildtime from a mirror of https://github.com/lloyd/yajl and statically linked in.

n) Textbooks and academic papers.

Some algorithms and data structures in this project were derived from the following textbooks and other academic sources:

* Introduction to Algorithms, Second Edition.  Cormen, Leiserson, Rivest.  2001.
* Algorithms in C, Parts 1-4: Fundamentals, Data Structures, Sorting, Searching, Third Edition. Sedgewick. 1997.
* Fundamental Algorithms. The Art of Computer Programming. 1, Second Edition.  Knuth. 1997.
* Hashed and Hierarchical Timing Wheels: Efficient Data Structures for Implementing a Timer Facility. Varghese, Lauck. 1997.
* Many articles published by Jonathan Corbet on lwn.net informed use of epoll and other system calls.
* Additional sources authored by Richard Stevens, John Lakos, Ulrich Drepper, Bjarne Stroustrup, Brian Kernighan, Dennis Ritchie, Eric Rescorla, and various IETF RFC authors were also referred to here and there.

