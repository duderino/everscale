cmake_minimum_required(VERSION 3.5)

include(FetchContent)
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

FetchContent_Declare(
        boringssl
        GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
        GIT_TAG fips-20190808
)

#FetchContent_MakeAvailable(boringssl)

FetchContent_GetProperties(boringssl)
if (NOT boringssl_POPULATED)
    FetchContent_Populate(boringssl)
    add_subdirectory(${boringssl_SOURCE_DIR} ${boringssl_BINARY_DIR})
    include_directories(${boringssl_SOURCE_DIR}/include)
endif ()

#
# gRPC
#

#FetchContent_Declare(
#        gRPC
#        GIT_REPOSITORY https://github.com/grpc/grpc
#        GIT_TAG        v1.28.0
#)
#FetchContent_MakeAvailable(gRPC)

