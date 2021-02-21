cmake_minimum_required(VERSION 3.5)
project(control-plane VERSION ${VERSION} LANGUAGES CXX)

file(GLOB_RECURSE LOCAL_SOURCES source/*.cpp include/*.h)
set(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

include(dependencies.cmake)

set(PROTO_FILES
        ${GOGO_PROTOBUF_PROTOS_DIR}/gogoproto/gogo.proto
        ${PROTOBUF_PROTOS_DIR}/google/protobuf/descriptor.proto
        ${PROTOBUF_PROTOS_DIR}/google/protobuf/duration.proto
        ${PROTOBUF_PROTOS_DIR}/google/protobuf/wrappers.proto
        ${GOOGLE_APIS_PROTOS_DIR}/google/api/field_behavior.proto
        ${ISTIO_API_PROTOS_DIR}/networking/v1beta1/service_entry.proto
        ${ISTIO_API_PROTOS_DIR}/networking/v1beta1/destination_rule.proto
        ${ISTIO_API_PROTOS_DIR}/networking/v1beta1/gateway.proto
        ${ISTIO_API_PROTOS_DIR}/networking/v1beta1/sidecar.proto
        ${ISTIO_API_PROTOS_DIR}/networking/v1beta1/virtual_service.proto
        ${ISTIO_API_PROTOS_DIR}/networking/v1beta1/workload_entry.proto
        )

set(GENERATED_DIR ${PROJECT_SOURCE_DIR}/generated)
set(GENERATED_FILES
        ${GENERATED_DIR}/google/protobuf/wrappers.grpc.pb.cc
        ${GENERATED_DIR}/google/protobuf/descriptor.pb.cc
        ${GENERATED_DIR}/google/protobuf/duration.pb.cc
        ${GENERATED_DIR}/google/protobuf/descriptor.grpc.pb.cc
        ${GENERATED_DIR}/google/protobuf/duration.grpc.pb.cc
        ${GENERATED_DIR}/google/protobuf/wrappers.pb.cc
        ${GENERATED_DIR}/google/api/field_behavior.pb.cc
        ${GENERATED_DIR}/google/api/field_behavior.grpc.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/destination_rule.grpc.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/sidecar.grpc.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/destination_rule.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/service_entry.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/virtual_service.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/gateway.grpc.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/service_entry.grpc.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/gateway.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/workload_entry.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/virtual_service.grpc.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/sidecar.pb.cc
        ${GENERATED_DIR}/networking/v1beta1/workload_entry.grpc.pb.cc
        ${GENERATED_DIR}/gogoproto/gogo.pb.cc
        ${GENERATED_DIR}/gogoproto/gogo.grpc.pb.cc
        )

file(MAKE_DIRECTORY ${GENERATED_DIR})
add_custom_command(
        OUTPUT ${GENERATED_FILES}
        COMMAND ${PROTOC_PATH}
        ARGS
        --grpc_out ${GENERATED_DIR}
        --cpp_out ${GENERATED_DIR}
        -I ${ISTIO_API_PROTOS_DIR}
        -I ${PROTOBUF_PROTOS_DIR}
        -I ${GOOGLE_APIS_PROTOS_DIR}
        -I ${GOGO_PROTOBUF_PROTOS_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PATH}
        ${PROTO_FILES}
        DEPENDS ${PROTO_FILES} grpc api
)

include_directories(${GENERATED_DIR} ${GRPC_INC_DIR} ${PROTOBUF_INC_DIR})

add_library(istio_protos_lib ${GENERATED_FILES})

