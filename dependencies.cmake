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
        PREFIX bssl
        SOURCE_DIR ${bssl_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
        GIT_TAG fips-20190808
        CMAKE_CACHE_ARGS -DFIPS:BOOL=1
        INSTALL_COMMAND ""
        )

add_library(bssl_ssl STATIC IMPORTED GLOBAL)
add_library(bssl_crypto STATIC IMPORTED GLOBAL)
include_directories(${PROJECT_SOURCE_DIR}/bssl/src/bssl/include)

set_property(TARGET bssl_ssl
        PROPERTY IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/bssl/src/bssl/ssl/libssl.a
        )
set_property(TARGET bssl_crypto
        PROPERTY IMPORTED_LOCATION
        ${PROJECT_SOURCE_DIR}/bssl/src/bssl/crypto/libcrypto.a
        )
