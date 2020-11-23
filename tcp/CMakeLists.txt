cmake_minimum_required(VERSION 3.5)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

add_subdirectory(client)
add_subdirectory(server)



