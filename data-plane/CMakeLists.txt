cmake_minimum_required(VERSION 3.5)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

add_subdirectory(config)
add_subdirectory(http-test-common)
add_subdirectory(http-common)
add_subdirectory(http1)
add_subdirectory(multiplexers)
add_subdirectory(filter)
add_subdirectory(loadgen)
add_subdirectory(origin)
add_subdirectory(proxy)

set(ALL_TESTS ${ALL_TESTS} PARENT_SCOPE)
set(PROFRAW_FILES "${PROFRAW_FILES}" PARENT_SCOPE)
