cmake_minimum_required(VERSION 3.5)
project(control VERSION ${VERSION} LANGUAGES CXX)

file(GLOB_RECURSE LOCAL_SOURCES source/*.cpp include/*.h)
set(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

include(dependencies.cmake)

set(PROTO_FILES
        ${XDS_PROTOS_DIR}/envoy/service/listener/v3/lds.proto
        ${XDS_PROTOS_DIR}/envoy/service/discovery/v3/discovery.proto
        ${XDS_PROTOS_DIR}/envoy/service/discovery/v3/ads.proto
        ${XDS_PROTOS_DIR}/envoy/config/core/v3/base.proto
        ${XDS_PROTOS_DIR}/envoy/config/core/v3/address.proto
        ${XDS_PROTOS_DIR}/envoy/config/core/v3/socket_option.proto
        ${XDS_PROTOS_DIR}/envoy/config/core/v3/backoff.proto
        ${XDS_PROTOS_DIR}/envoy/config/core/v3/http_uri.proto
        ${XDS_PROTOS_DIR}/envoy/type/v3/percent.proto
        ${XDS_PROTOS_DIR}/envoy/type/v3/semantic_version.proto
        ${XDS_PROTOS_DIR}/envoy/annotations/resource.proto
        ${UDPA_PROTOS_DIR}/udpa/annotations/status.proto
        ${UDPA_PROTOS_DIR}/udpa/annotations/versioning.proto
        ${UDPA_PROTOS_DIR}/udpa/annotations/migrate.proto
        ${PROTOC_GEN_VALIDATE_PROTOS_DIR}/validate/validate.proto
        ${GOOGLE_APIS_PROTOS_DIR}/google/rpc/status.proto
        ${GOOGLE_APIS_PROTOS_DIR}/google/api/annotations.proto
        ${GOOGLE_APIS_PROTOS_DIR}/google/api/http.proto
        )

set(GENERATED_DIR ${PROJECT_SOURCE_DIR}/generated)
set(GENERATED_FILES
        ${GENERATED_DIR}/google/rpc/status.pb.cc
        ${GENERATED_DIR}/google/rpc/status.grpc.pb.cc
        ${GENERATED_DIR}/google/api/annotations.pb.cc
        ${GENERATED_DIR}/google/api/http.grpc.pb.cc
        ${GENERATED_DIR}/google/api/http.pb.cc
        ${GENERATED_DIR}/google/api/annotations.grpc.pb.cc
        ${GENERATED_DIR}/udpa/annotations/versioning.pb.cc
        ${GENERATED_DIR}/udpa/annotations/migrate.pb.cc
        ${GENERATED_DIR}/udpa/annotations/status.pb.cc
        ${GENERATED_DIR}/udpa/annotations/migrate.grpc.pb.cc
        ${GENERATED_DIR}/udpa/annotations/versioning.grpc.pb.cc
        ${GENERATED_DIR}/udpa/annotations/status.grpc.pb.cc
        ${GENERATED_DIR}/validate/validate.pb.cc
        ${GENERATED_DIR}/validate/validate.grpc.pb.cc
        ${GENERATED_DIR}/envoy/annotations/resource.pb.cc
        ${GENERATED_DIR}/envoy/annotations/resource.grpc.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/backoff.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/address.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/address.grpc.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/http_uri.grpc.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/socket_option.grpc.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/base.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/http_uri.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/socket_option.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/base.grpc.pb.cc
        ${GENERATED_DIR}/envoy/config/core/v3/backoff.grpc.pb.cc
        ${GENERATED_DIR}/envoy/service/listener/v3/lds.pb.cc
        ${GENERATED_DIR}/envoy/service/listener/v3/lds.grpc.pb.cc
        ${GENERATED_DIR}/envoy/service/discovery/v3/discovery.grpc.pb.cc
        ${GENERATED_DIR}/envoy/service/discovery/v3/discovery.pb.cc
        ${GENERATED_DIR}/envoy/service/discovery/v3/ads.pb.cc
        ${GENERATED_DIR}/envoy/service/discovery/v3/ads.grpc.pb.cc
        ${GENERATED_DIR}/envoy/type/v3/percent.grpc.pb.cc
        ${GENERATED_DIR}/envoy/type/v3/semantic_version.grpc.pb.cc
        ${GENERATED_DIR}/envoy/type/v3/semantic_version.pb.cc
        ${GENERATED_DIR}/envoy/type/v3/percent.pb.cc)

file(MAKE_DIRECTORY ${GENERATED_DIR})
add_custom_command(
        OUTPUT ${GENERATED_FILES}
        COMMAND ${PROTOC_PATH}
        ARGS
        --grpc_out ${GENERATED_DIR}
        --cpp_out ${GENERATED_DIR}
        -I ${XDS_PROTOS_DIR}
        -I ${UDPA_PROTOS_DIR}
        -I ${PROTOBUF_PROTOS_DIR}
        -I ${PROTOC_GEN_VALIDATE_PROTOS_DIR}
        -I ${GRPC_PROTOS_DIR}
        -I ${GOOGLE_APIS_PROTOS_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PATH}
        ${PROTO_FILES}
        DEPENDS ${PROTO_FILES} grpc xds
)

include_directories(${GENERATED_DIR} ${GRPC_INC_DIR} ${PROTOBUF_INC_DIR})

add_library(xds_protos_lib ${GENERATED_FILES})

add_executable(ListenerDiscoveryService source/ListenerDiscoveryService.cpp)
target_link_libraries(ListenerDiscoveryService
        -pthread
        xds_protos_lib
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

add_executable(ListenerDiscoveryClient source/ListenerDiscoveryClient.cpp)
target_link_libraries(ListenerDiscoveryClient
        -pthread
        xds_protos_lib
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
