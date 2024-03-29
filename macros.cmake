cmake_minimum_required(VERSION 3.5)

#
# googletest and unit-tf test macros
#

macro(add_gtest NAME INCS LIBS CWD TIMEOUT)
    list(APPEND TESTS "${PROJECT_SOURCE_DIR}/${NAME}")
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} gtest gmock gtest_main ${LIBS})
    gtest_discover_tests(${NAME}
            WORKING_DIRECTORY ${CWD}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CWD}" TIMEOUT ${TIMEOUT} ENVIRONMENT "LLVM_PROFILE_FILE=%m-%p.profraw"
            NO_PRETTY_TYPES
            NO_PRETTY_VALUES
            )
    set_target_properties(${NAME} PROPERTIES FOLDER tests)
endmacro()

macro(add_unit_test NAME INCS LIBS CWD TIMEOUT)
    list(APPEND TESTS "${PROJECT_SOURCE_DIR}/${NAME}")
    add_executable(${NAME} ${ARGN})
    target_compile_options(${NAME} PUBLIC ${CPP_FLAGS} -DESTF_USE_RESULT_COLLECTOR -DUSE_SMART_POINTER_DEBUGGER)
    target_link_libraries(${NAME} ${LIBS})
    target_include_directories(${NAME} PRIVATE ${INCS})
    # By default ctest eats stderr/out which makes debugging test failures inconvenient.
    # To see stderr/stdout on test failure, do a:
    #   CTEST_OUTPUT_ON_FAILURE=1 make test
    # Or add "export CTEST_OUTPUT_ON_FAILURE=1" to your .bashrc
    add_test(NAME ${NAME}
            COMMAND ${NAME}
            WORKING_DIRECTORY ${CWD})
    set_tests_properties(${NAME} PROPERTIES
            TIMEOUT ${TIMEOUT}
            ENVIRONMENT "LLVM_PROFILE_FILE=%m-%p.profraw"
            )
endmacro()

macro(add_mocha_test NAME CWD TIMEOUT)
    math(EXPR millis "${TIMEOUT} * 1000")
    add_test(NAME ${NAME}
            COMMAND env BUILD_TYPE=${CMAKE_BUILD_TYPE} ${MOCHA} -t ${millis} ${ARGN}
            WORKING_DIRECTORY ${CWD})
endmacro()

