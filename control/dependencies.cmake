cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

#
# gRPC dependencies.  BoringSSL is built so it can be compiled with -fPIC and created in a shared lib for use by
# the gRPC C++ protoc plugin.  This build of BoringSSL is not used at runtime.   Also protobuf and gRPC are forced
# to use the same zlib dependency.
#

if (DEFINED BSSL_TAG)
    message(STATUS "BoringSSL Tag: ${BSSL_TAG}")
else()
    message(FATAL_ERROR "BSSL_TAG not defined")
endif()

ExternalProject_Add(bssl_dynamic
        PREFIX third_party
        SOURCE_DIR ${bssl_dynamic_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
        GIT_TAG ${BSSL_TAG}
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DFIPS:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON
        INSTALL_COMMAND mkdir -p lib && cp ssl/libssl.so lib && cp crypto/libcrypto.so lib
        )

set(BSSL_DYNAMIC_ROOT_DIR ${PROJECT_SOURCE_DIR}/third_party/src/bssl_dynamic)
set(BSSL_DYNAMIC_INCLUDE_DIR ${BSSL_DYNAMIC_ROOT_DIR}/include)
set(BSSL_DYNAMIC_LIBRARIES ${BSSL_DYNAMIC_ROOT_DIR}/lib)

ExternalProject_Add(zlib
        PREFIX third_party
        SOURCE_DIR "${zlib_SOURCE_DIR}"
        BUILD_IN_SOURCE 1
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.2.11
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Release
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
        -DgRPC_BUILD_GRPC_CSHARP_PLUGIN:BOOL=OFF
        -DgRPC_BUILD_GRPC_NODE_PLUGIN:BOOL=OFF
        -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN:BOOL=OFF
        -DgRPC_BUILD_GRPC_PHP_PLUGIN:BOOL=OFF
        -DgRPC_BUILD_GRPC_PYTHON_PLUGIN:BOOL=OFF
        -DgRPC_BUILD_GRPC_RUBY_PLUGIN:BOOL=OFF
        -DgRPC_INSTALL:BOOL=OFF
        -DgRPC_BUILD_TESTS:BOOL=OFF
        -DgRPC_ZLIB_PROVIDER:STRING=package
        -DZLIB_ROOT:STRING=${ZLIB_DIR}
        -DgRPC_PROTOBUF_PROVIDER:STRING=package
        -DgRPC_PROTOBUF_PACKAGE_TYPE:STRING=CONFIG
        -DProtobuf_DIR:PATH=${PROTOBUF_DIR}
        -DgRPC_SSL_PROVIDER=package
        -DOPENSSL_ROOT_DIR:PATH=${BSSL_DYNAMIC_ROOT_DIR}
        -DOPENSSL_INCLUDE_DIR:PATH=${BSSL_DYNAMIC_INCLUDE_DIR}
        -DOPENSSL_LIBRARIES:PATH=${BSSL_DYNAMIC_LIBRARIES}
        INSTALL_COMMAND ""
        DEPENDS protobuf bssl_dynamic
        )

#
# xDS API
#

ExternalProject_Add(xds
        PREFIX third_party
        SOURCE_DIR "${xds_SOURCE_DIR}"
        GIT_REPOSITORY https://github.com/envoyproxy/data-plane-api.git
        GIT_TAG 3d7c538ed0c11dee176b23a93f1e4c08817ddf3c
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        )

