cmake_minimum_required(VERSION 3.5)
project(config VERSION ${VERSION} LANGUAGES CXX)

set(SOURCE_FILES
		source/ESConfigIngest.cpp
		source/ESEntity.cpp
		source/ESAction.cpp
		source/ESCondition.cpp
		source/ESRule.cpp
        )

add_library(config STATIC ${SOURCE_FILES})

target_include_directories(config PRIVATE
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        )

target_include_directories(config PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        )

# Unit tests

set(TEST_INCS
        "${PROJECT_SOURCE_DIR}/source"
        "${PROJECT_SOURCE_DIR}/tests"
        "${PROJECT_SOURCE_DIR}/../../base/include"
        "${PROJECT_SOURCE_DIR}/../../unit-tf/include"
        "${CMAKE_SOURCE_DIR}/data-plane/config/include"
        )

SET(TEST_LIBS
        -pthread
        config
        unit-tf
        base
		yajl
        )

add_gtest(action-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESActionTest.cpp tests/ESConfigTest.cpp)
add_gtest(entity-gtest "${TEST_INCS}" "${TEST_LIBS}" ${PROJECT_SOURCE_DIR}/tests 300 tests/ESEntityTest.cpp tests/ESConfigTest.cpp)

# For global code coverage report

set(ALL_TESTS ${ALL_TESTS} ${TESTS} PARENT_SCOPE)
set(PROFRAW_FILES "${PROFRAW_FILES}" "${PROJECT_SOURCE_DIR}/tests/*.profraw" PARENT_SCOPE)
