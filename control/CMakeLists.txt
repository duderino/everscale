cmake_minimum_required(VERSION 3.5)
project(control VERSION ${VERSION} LANGUAGES CXX)

include(dependencies.cmake)

SET(PROTOC_PATH ${PROJECT_SOURCE_DIR}/third_party/src/protobuf/cmake/protoc)
SET(PROTOBUF_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/protobuf/cmake)
SET(GRPC_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/grpc)
SET(GRPC_CPP_PLUGIN_PATH ${PROJECT_SOURCE_DIR}/third_party/src/grpc/grpc_cpp_plugin)
SET(ABSL_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/grpc/third_party/abseil-cpp/absl)
SET(ZLIB_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/zlib)
SET(RE2_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/grpc/third_party/re2)
SET(CARES_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party/src/grpc/third_party/cares/cares/lib)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

# Proto file
get_filename_component(hw_proto "protos/helloworld.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
SET(hw_proto_srcs "${PROJECT_SOURCE_DIR}/generated/helloworld.pb.cc")
SET(hw_proto_hdrs "${PROJECT_SOURCE_DIR}/generated/helloworld.pb.h")
SET(hw_grpc_srcs "${PROJECT_SOURCE_DIR}/generated/helloworld.grpc.pb.cc")
SET(hw_grpc_hdrs "${PROJECT_SOURCE_DIR}/generated/helloworld.grpc.pb.h")


add_custom_command(
        OUTPUT "${hw_proto_srcs}" "${hw_proto_hdrs}" "${hw_grpc_srcs}" "${hw_grpc_hdrs}"
        COMMAND ${PROTOC_PATH}
        ARGS --grpc_out "${PROJECT_SOURCE_DIR}/generated"
        --cpp_out "${PROJECT_SOURCE_DIR}/generated"
        -I "${hw_proto_path}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN_PATH}"
        "${hw_proto}"
        DEPENDS "${hw_proto}")

include_directories(
        ${PROJECT_SOURCE_DIR}/generated
        ${PROJECT_SOURCE_DIR}/third_party/src/grpc/include
        ${PROJECT_SOURCE_DIR}/third_party/src/protobuf/src
)

# hw_grpc_proto
add_library(hw_grpc_proto
        ${hw_grpc_srcs}
        ${hw_grpc_hdrs}
        ${hw_proto_srcs}
        ${hw_proto_hdrs})
target_link_libraries(hw_grpc_proto
        ${GRPC_LIB_DIR}/libgrpc++_reflection.a
        ${GRPC_LIB_DIR}/libgrpc++.a
        ${PROTOBUF_LIB_DIR}//libprotobuf.a)

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
