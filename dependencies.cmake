cmake_minimum_required(VERSION 3.5)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/third_party/src/)

include(ExternalProject)

#
# clang-format for make format
#

find_program(CLANG_FORMAT "clang-format")

if (NOT CLANG_FORMAT)
    message(FATAL_ERROR "clang-format not found.  Try: sudo apt install clang-format")
else ()
    message(STATUS "clang-format found: ${CLANG_FORMAT}")
endif ()

#
# mocha for nodejs-based interop tests
#

find_program(MOCHA "mocha")

if (NOT MOCHA)
    message(FATAL_ERROR "mocha not found.  Try: sudo apt install mocha")
else ()
    message(STATUS "mocha found: ${MOCHA}")
endif ()

#
# googletest
#

set(GTEST_DIR ${FETCHCONTENT_BASE_DIR}/googletest-src)

#if (EXISTS ${GTEST_DIR})
#    message(STATUS "${GTEST_DIR} exists, skipping rebuild")
#    set(FETCHCONTENT_SOURCE_DIR_GOOGLETEST ${GTEST_DIR})
#else ()
    FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
            )
#endif ()

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

set(BSSL_TAG 0.20250701.0)  # If you change this, update the patch in third_party/patch/bssl/index.patch. If you remove the patch, remove the PATCH_COMMAND below
set(BSSL_ROOT_DIR ${CMAKE_SOURCE_DIR}/third_party/src/bssl)
set(BSSL_INCLUDE_DIR ${BSSL_ROOT_DIR}/include)
set(BSSL_LIB_DIR ${BSSL_ROOT_DIR})

# This EXISTS checks with no-op custom target can be removed once
# https://gitlab.kitware.com/cmake/cmake/-/issues/16419# is fixed.

if (EXISTS ${BSSL_LIB_DIR}/libssl.a)
    add_custom_target(bssl
            COMMAND echo "${BSSL_LIB_DIR}/libssl.a exists, skipping rebuild")
else ()
    ExternalProject_Add(bssl
            PREFIX third_party
            SOURCE_DIR ${bssl_SOURCE_DIR}
            BUILD_IN_SOURCE 1
            GIT_REPOSITORY https://boringssl.googlesource.com/boringssl
            GIT_TAG ${BSSL_TAG}
	    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DFIPS:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=OFF "-DCMAKE_C_FLAGS=-w" "-DCMAKE_CXX_FLAGS=-w"
	    #INSTALL_COMMAND mkdir -p lib && cp libssl.a lib && cp libcrypto.a lib
            UPDATE_COMMAND ""
	    #PATCH_COMMAND patch -p1 < ../../patch/bssl/index.patch
    )
endif ()

include_directories(${BSSL_INCLUDE_DIR})

add_library(bssl_ssl STATIC IMPORTED GLOBAL)
set_property(TARGET bssl_ssl
        PROPERTY IMPORTED_LOCATION
        ${BSSL_LIB_DIR}/libssl.a
        )

add_library(bssl_crypto STATIC IMPORTED GLOBAL)
set_property(TARGET bssl_crypto
        PROPERTY IMPORTED_LOCATION
        ${BSSL_LIB_DIR}/libcrypto.a
        )

#
# yajl JSON SAX parser
#

set(YAJL_ROOT_DIR ${PROJECT_SOURCE_DIR}/third_party/src/yajl_parser)
set(YAJL_INCLUDE_DIR ${YAJL_ROOT_DIR}/include)
set(YAJL_LIB_DIR ${YAJL_ROOT_DIR}/lib)

if (EXISTS ${YAJL_INCLUDE_DIR}/yajl/yajl_parse.h)
    add_custom_target(yajl_parser COMMAND echo "${YAJL_INCLUDE_DIR} exists, skipping rebuild")
else ()
    ExternalProject_Add(yajl_parser
            PREFIX third_party
            SOURCE_DIR ${yajl_SOURCE_DIR}
            BUILD_IN_SOURCE 1
            GIT_REPOSITORY https://github.com/duderino/yajl.git
            GIT_TAG 5e3a7856e643b4d6410ddc3f84bc2f38174f2872
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
            INSTALL_COMMAND mkdir -p lib && cp yajl-2.1.1/lib/libyajl_s.a lib/libyajl.a && mkdir -p include/yajl && cp src/api/yajl_common.h src/api/yajl_gen.h src/api/yajl_parse.h src/api/yajl_tree.h include/yajl
            UPDATE_COMMAND ""
            )
endif ()

include_directories(${YAJL_INCLUDE_DIR})

add_library(yajl STATIC IMPORTED GLOBAL)
set_property(TARGET yajl
        PROPERTY IMPORTED_LOCATION
        ${YAJL_LIB_DIR}/libyajl.a
        )
