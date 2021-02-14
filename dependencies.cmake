cmake_minimum_required(VERSION 3.5)

include(FetchContent)
include(ExternalProject)
set(FETCHCONTENT_QUIET OFF)

#
# clang-format
#

find_program(CLANG_FORMAT "clang-format")

if (NOT CLANG_FORMAT)
    message(STATUS "clang-format not found.")
else ()
    message(STATUS "clang-format found: ${CLANG_FORMAT}")
endif ()

#
# googletest
#

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.10.x
)

FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    include_directories(${gtest_SOURCE_DIR}/include)
endif ()

include(GoogleTest)

mark_as_advanced(
        BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
        gmock_build_tests gtest_build_samples gtest_build_tests
        gtest_enable_pthreads
)

#
# BoringSSL
#

ExternalProject_Add(bssl
        PREFIX third_party
        SOURCE_DIR ${bssl_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
        GIT_TAG fips-20190808
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DFIPS:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF
        INSTALL_COMMAND ""
        )

set(BSSL_ROOT_DIR ${PROJECT_SOURCE_DIR}/third_party/src/bssl)
add_library(bssl_ssl STATIC IMPORTED GLOBAL)
add_library(bssl_crypto STATIC IMPORTED GLOBAL)
include_directories(${BSSL_ROOT_DIR}/include)

set_property(TARGET bssl_ssl
        PROPERTY IMPORTED_LOCATION
        ${BSSL_ROOT_DIR}/ssl/libssl.a
        )
set_property(TARGET bssl_crypto
        PROPERTY IMPORTED_LOCATION
        ${BSSL_ROOT_DIR}/crypto/libcrypto.a
        )

#
# gRPC.  Note that zlib and protobuf are transitive dependencies of gRPC, but are exposed here to ensure that protobuf
# internally uses zlib and uses the same zlib as gRPC.  Note also that gRPC's transitive use of BoringSSL is replaced
# by the BoringSSL used by this top-level project (one BoringSSL to rule them all).
#

ExternalProject_Add(zlib
        PREFIX third_party
        SOURCE_DIR "${zlib_SOURCE_DIR}"
        BUILD_IN_SOURCE 1
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.2.11
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
        INSTALL_COMMAND ""
        )

set(ZLIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/zlib)

ExternalProject_Add(protobuf
        PREFIX third_party
        SOURCE_DIR ${protobuf_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG v3.14.0
        SOURCE_SUBDIR cmake
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -Dprotobuf_WITH_ZLIB:BOOL=ON
        -DZLIB_INCLUDE_DIR=${ZLIB_DIR}
        -DZLIB_LIBRARY=${ZLIB_DIR}//libz.a
        -Dprotobuf_BUILD_TESTS:BOOL=OFF
        -Dprotobuf_BUILD_EXAMPLES:BOOL=OFF
        -Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
        INSTALL_COMMAND ""
        DEPENDS zlib
        )

set(PROTOBUF_DIR ${PROJECT_SOURCE_DIR}/third_party/src/protobuf/cmake/lib/cmake/protobuf)

ExternalProject_Add(grpc
        PREFIX third_party
        SOURCE_DIR "${grpc_SOURCE_DIR}"
        BUILD_IN_SOURCE 1
        GIT_REPOSITORY https://github.com/grpc/grpc.git
        GIT_TAG v1.35.0
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
        -DgRPC_ZLIB_PROVIDER:STRING=package
        -DZLIB_ROOT:STRING=${ZLIB_DIR}
        -DgRPC_PROTOBUF_PROVIDER:STRING=package
        -DgRPC_PROTOBUF_PACKAGE_TYPE:STRING=CONFIG
        -DProtobuf_DIR:PATH=${PROTOBUF_DIR}
        -DgRPC_SSL_PROVIDER=package
        -DOPENSSL_ROOT_DIR:PATH=${BSSL_ROOT_DIR}
        -DgRPC_INSTALL:BOOL=OFF
        -DgRPC_BUILD_TESTS:BOOL=OFF
        INSTALL_COMMAND ""
        DEPENDS protobuf bssl
        )
