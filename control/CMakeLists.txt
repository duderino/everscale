cmake_minimum_required(VERSION 3.5)
project(control VERSION ${VERSION} LANGUAGES CXX)

file(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
set(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

include(dependencies.cmake)

set(PROTOS_DIR ${PROJECT_SOURCE_DIR}/protos)
set(PROTO_FILES
        ${PROTOS_DIR}/helloworld.proto)

set(GENERATED_DIR ${PROJECT_SOURCE_DIR}/generated)
set(GENERATED_FILES
        ${GENERATED_DIR}/helloworld.pb.cc
        ${GENERATED_DIR}/helloworld.grpc.pb.cc)

add_custom_command(
        OUTPUT ${GENERATED_FILES}
        COMMAND ${PROTOC_PATH}
        ARGS
        --grpc_out ${GENERATED_DIR}
        --cpp_out ${GENERATED_DIR}
        -I ${PROTOS_DIR}
        --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_PATH}
        ${PROTO_FILES}
        DEPENDS ${PROTO_FILES} grpc
)

include_directories(${GENERATED_DIR} ${GRPC_INC_DIR} ${PROTOBUF_INC_DIR})

# hw_grpc_proto
add_library(hw_grpc_proto ${GENERATED_FILES})

add_executable(GreeterClient source/GreeterClient.cpp)
target_link_libraries(GreeterClient
        hw_grpc_proto
        ${GRPC_LIB_DIR}/libgrpc++_reflection.a
        ${GRPC_LIB_DIR}/libgrpc++.a
        ${GRPC_LIB_DIR}/libgrpc.a
        ${GRPC_LIB_DIR}/libgpr.a
        ${GRPC_LIB_DIR}/libupb.a
        ${GRPC_LIB_DIR}/libaddress_sorting.a
        ${PROTOBUF_LIB_DIR}//libprotobuf.a
        ${BSSL_LIBRARIES}/libssl.a
        ${BSSL_LIBRARIES}/libcrypto.a
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

add_executable(GreeterServer source/GreeterServer.cpp)
target_link_libraries(GreeterServer
        hw_grpc_proto
        ${GRPC_LIB_DIR}/libgrpc++_reflection.a
        ${GRPC_LIB_DIR}/libgrpc++.a
        ${GRPC_LIB_DIR}/libgrpc.a
        ${GRPC_LIB_DIR}/libgpr.a
        ${GRPC_LIB_DIR}/libupb.a
        ${GRPC_LIB_DIR}/libaddress_sorting.a
        ${PROTOBUF_LIB_DIR}//libprotobuf.a
        ${BSSL_LIBRARIES}/libssl.a
        ${BSSL_LIBRARIES}/libcrypto.a
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
