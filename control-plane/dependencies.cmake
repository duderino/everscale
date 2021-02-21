cmake_minimum_required(VERSION 3.5)

include(ExternalProject)

#
# gRPC dependencies.  BoringSSL is built so it can be compiled with -fPIC and created in a shared lib for use by
# the gRPC C++ protoc plugin.  This build of BoringSSL is not used at runtime.   Also protobuf and gRPC are forced
# to use the same zlib dependency.
#

if (DEFINED BSSL_TAG)
    message(STATUS "BoringSSL Tag: ${BSSL_TAG}")
else ()
    message(FATAL_ERROR "BSSL_TAG not defined")
endif ()

set(BSSL_DYNAMIC_ROOT_DIR ${PROJECT_SOURCE_DIR}/third_party/src/bssl_dynamic)
set(BSSL_DYNAMIC_INCLUDE_DIR ${BSSL_DYNAMIC_ROOT_DIR}/include)
set(BSSL_DYNAMIC_LIBRARIES ${BSSL_DYNAMIC_ROOT_DIR}/lib)

# All these EXISTS checks with no-op custom targets can be removed once
# https://gitlab.kitware.com/cmake/cmake/-/issues/16419# is fixed.

if (EXISTS ${BSSL_DYNAMIC_ROOT_DIR})
    add_custom_target(bssl_dynamic
            COMMAND echo "${BSSL_DYNAMIC_ROOT_DIR} exists, skipping rebuild")
else ()
    ExternalProject_Add(bssl_dynamic
            PREFIX third_party
            SOURCE_DIR ${bssl_dynamic_SOURCE_DIR}
            BUILD_IN_SOURCE 1
            GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
            GIT_TAG ${BSSL_TAG}
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DFIPS:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=ON
            INSTALL_COMMAND mkdir -p lib && cp ssl/libssl.so lib && cp crypto/libcrypto.so lib
            UPDATE_COMMAND ""
            )
endif ()

set(ZLIB_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/zlib)
set(ZLIB_INC_DIR ${PROJECT_SOURCE_DIR}/third_party/src/zlib)

if (EXISTS ${ZLIB_LIB_DIR})
    add_custom_target(zlib
            COMMAND echo "${ZLIB_LIB_DIR} exists, skipping rebuild")
else ()
    ExternalProject_Add(zlib
            PREFIX third_party
            SOURCE_DIR "${zlib_SOURCE_DIR}"
            BUILD_IN_SOURCE 1
            GIT_REPOSITORY https://github.com/madler/zlib.git
            GIT_TAG v1.2.11
            CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=Release
            INSTALL_COMMAND ""
            UPDATE_COMMAND ""
            )
endif ()

set(PROTOBUF_DIR ${PROJECT_SOURCE_DIR}/third_party/src/protobuf)
set(PROTOC_PATH ${PROTOBUF_DIR}/cmake/protoc)
set(PROTOBUF_LIB_DIR ${PROTOBUF_DIR}/cmake)
set(PROTOBUF_INC_DIR ${PROTOBUF_DIR}/src)
set(PROTOBUF_PROTOS_DIR ${PROTOBUF_DIR}/src)
set(PROTOBUF_CMAKE_DIR ${PROTOBUF_DIR}//cmake/lib/cmake/protobuf)

if (EXISTS ${PROTOC_PATH})
    add_custom_target(protobuf
            COMMAND echo "${PROTOC_PATH} exists, skipping protobuf rebuild")
else ()
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
            -DZLIB_INCLUDE_DIR=${ZLIB_INC_DIR}
            -DZLIB_LIBRARY=${ZLIB_LIB_DIR}/libz.a
            -Dprotobuf_BUILD_TESTS:BOOL=OFF
            -Dprotobuf_BUILD_EXAMPLES:BOOL=OFF
            -Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
            INSTALL_COMMAND ""
            UPDATE_COMMAND ""
            DEPENDS zlib
            )
endif ()

set(GRPC_DIR ${PROJECT_SOURCE_DIR}/third_party/src/grpc)
set(GRPC_CPP_PLUGIN_PATH ${GRPC_DIR}/grpc_cpp_plugin)
set(GRPC_INC_DIR ${GRPC_DIR}/include)
set(GRPC_LIB_DIR ${GRPC_DIR})
set(GRPC_PROTOS_DIR ${GRPC_DIR}/src/proto)
set(ABSL_LIB_DIR ${GRPC_DIR}/third_party/abseil-cpp/absl)
set(RE2_LIB_DIR ${GRPC_DIR}/third_party/re2)
set(CARES_LIB_DIR ${GRPC_DIR}/third_party/cares/cares/lib)


if (EXISTS ${GRPC_CPP_PLUGIN_PATH})
    add_custom_target(grpc
            COMMAND echo "${GRPC_CPP_PLUGIN_PATH} exists, skipping rebuild")
else ()
    ExternalProject_Add(grpc
            PREFIX third_party
            SOURCE_DIR "${grpc_SOURCE_DIR}"
            BUILD_IN_SOURCE 1t
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
            -DZLIB_ROOT:STRING=${ZLIB_LIB_DIR}
            -DgRPC_PROTOBUF_PROVIDER:STRING=package
            -DgRPC_PROTOBUF_PACKAGE_TYPE:STRING=CONFIG
            -DProtobuf_DIR:PATH=${PROTOBUF_CMAKE_DIR}
            -DgRPC_SSL_PROVIDER=package
            -DOPENSSL_ROOT_DIR:PATH=${BSSL_DYNAMIC_ROOT_DIR}
            -DOPENSSL_INCLUDE_DIR:PATH=${BSSL_DYNAMIC_INCLUDE_DIR}
            -DOPENSSL_LIBRARIES:PATH=${BSSL_DYNAMIC_LIBRARIES}
            INSTALL_COMMAND ""
            UPDATE_COMMAND ""
            DEPENDS protobuf bssl_dynamic
            )
endif ()

set(GRPC_STATIC_LIBS
        ${GRPC_LIB_DIR}/libgrpc++_reflection.a
        ${GRPC_LIB_DIR}/libgrpc++.a
        ${GRPC_LIB_DIR}/libgrpc.a
        ${GRPC_LIB_DIR}/libgpr.a
        ${GRPC_LIB_DIR}/libupb.a
        ${GRPC_LIB_DIR}/libaddress_sorting.a
        ${PROTOBUF_LIB_DIR}//libprotobuf.a
        ${BSSL_LIB_DIR}/libssl.a
        ${BSSL_LIB_DIR}/libcrypto.a
        ${ABSL_LIB_DIR}/status/libabsl_statusor.a
        ${ABSL_LIB_DIR}/synchronization/libabsl_synchronization.a
        ${ABSL_LIB_DIR}/time/libabsl_time.a
        ${ABSL_LIB_DIR}/time/libabsl_time_zone.a
        ${ABSL_LIB_DIR}/numeric/libabsl_int128.a
        ${ABSL_LIB_DIR}/status/libabsl_status.a
        ${ABSL_LIB_DIR}/debugging/libabsl_stacktrace.a
        ${ABSL_LIB_DIR}/debugging/libabsl_symbolize.a
        ${ABSL_LIB_DIR}/debugging/libabsl_demangle_internal.a
        ${ABSL_LIB_DIR}/debugging/libabsl_debugging_internal.a
        ${ABSL_LIB_DIR}/base/libabsl_raw_logging_internal.a
        ${ABSL_LIB_DIR}/base/libabsl_malloc_internal.a
        ${ABSL_LIB_DIR}/base/libabsl_spinlock_wait.a
        ${ABSL_LIB_DIR}/strings/libabsl_strings.a
        ${ABSL_LIB_DIR}/strings/libabsl_cord.a
        ${ABSL_LIB_DIR}/strings/libabsl_strings_internal.a
        ${ABSL_LIB_DIR}/strings/libabsl_str_format_internal.a
        ${ABSL_LIB_DIR}/types/libabsl_bad_optional_access.a
        ${ABSL_LIB_DIR}/base/libabsl_throw_delegate.a
        ${ABSL_LIB_DIR}/base/libabsl_base.a
        ${ZLIB_LIB_DIR}/libz.a
        ${RE2_LIB_DIR}/libre2.a
        ${CARES_LIB_DIR}/libcares.a
        )

#
# Istio API
#

ExternalProject_Add(googleapis
        PREFIX third_party
        SOURCE_DIR ${googleapis_SOURCE_DIR}
        GIT_REPOSITORY https://github.com/googleapis/googleapis.git
        GIT_TAG af8f71dae961ee036a0ac52a1965270149a6b2c9
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        )

set(GOOGLE_APIS_PROTOS_DIR ${PROJECT_SOURCE_DIR}/third_party/src/googleapis)

ExternalProject_Add(gogo-protobuf
        PREFIX third_party
        SOURCE_DIR ${gogo-protobuf_SOURCE_DIR}
        GIT_REPOSITORY https://github.com/gogo/protobuf.git
        GIT_TAG v1.3.2
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        )

set(GOGO_PROTOBUF_PROTOS_DIR ${PROJECT_SOURCE_DIR}/third_party/src/gogo-protobuf)

ExternalProject_Add(api
        PREFIX third_party
        SOURCE_DIR ${api_SOURCE_DIR}
        GIT_REPOSITORY https://github.com/istio/api.git
        GIT_TAG 1.9.0
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        DEPENDS grpc googleapis protobuf gogo-protobuf
        )

set(ISTIO_API_PROTOS_DIR ${PROJECT_SOURCE_DIR}/third_party/src/api)
