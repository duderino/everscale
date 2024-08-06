cmake_minimum_required(VERSION 3.5)
project(nodejs-tests VERSION ${VERSION} LANGUAGES CXX)

FILE(GLOB_RECURSE LOCAL_SOURCES . *.cpp *.h)
SET(FORMAT_FILES ${FORMAT_FILES} ${LOCAL_SOURCES} PARENT_SCOPE)

# The reason for the use of --exit is described in https://boneskull.com/mocha-v4-nears-release/#mochawontforceexit
# Mocha4 hangs on exit if any resources (files, etc) are not released by the script.  --exit forces exit anyways.
add_mocha_test(nodejs-test ${PROJECT_SOURCE_DIR} 300 --exit test.js)

