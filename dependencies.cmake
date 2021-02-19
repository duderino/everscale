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
        UPDATE_COMMAND ""
)

FetchContent_GetProperties(googletest)
if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    include_directories(${gtest_SOURCE_DIR}/include)
endif ()

#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
include(GoogleTest)

mark_as_advanced(
        BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS
        gmock_build_tests gtest_build_samples gtest_build_tests
        gtest_enable_pthreads
)

#
# BoringSSL
#

set(BSSL_TAG fips-20190808)
set(BSSL_ROOT_DIR ${CMAKE_SOURCE_DIR}/third_party/src/bssl)
set(BSSL_INCLUDE_DIR ${BSSL_ROOT_DIR}/include)
set(BSSL_LIBRARIES ${BSSL_ROOT_DIR}/lib)

# This EXISTS checks with no-op custom target can be removed once
# https://gitlab.kitware.com/cmake/cmake/-/issues/16419# is fixed.

if (EXISTS ${BSSL_LIBRARIES})
    add_custom_target(bssl
            COMMAND echo "${BSSL_LIBRARIES} exists, skipping rebuild")
else()
    ExternalProject_Add(bssl
            PREFIX third_party
            SOURCE_DIR ${bssl_SOURCE_DIR}
            BUILD_IN_SOURCE 1
            GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
            GIT_TAG ${BSSL_TAG}
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DFIPS:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=OFF
            INSTALL_COMMAND mkdir -p lib && cp ssl/libssl.a lib && cp crypto/libcrypto.a lib
            UPDATE_COMMAND ""
            )
endif()

include_directories(${BSSL_INCLUDE_DIR})

add_library(bssl_ssl STATIC IMPORTED GLOBAL)
set_property(TARGET bssl_ssl
        PROPERTY IMPORTED_LOCATION
        ${BSSL_LIBRARIES}/libssl.a
        )

add_library(bssl_crypto STATIC IMPORTED GLOBAL)
set_property(TARGET bssl_crypto
        PROPERTY IMPORTED_LOCATION
        ${BSSL_LIBRARIES}/libcrypto.a
        )


