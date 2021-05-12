cmake_minimum_required(VERSION 3.5)
project(nodejs-tests VERSION ${VERSION} LANGUAGES CXX)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

add_mocha_test(origin-test ${PROJECT_SOURCE_DIR} 300 origin_test.js)

