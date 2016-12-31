# 
# CMakeLists file for the test project.
# 

cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE TEST_HEADERS ${PROJECT_SOURCE_DIR}/test/include/*.hpp)
file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/src/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -O0 -g")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/CMake-codecov/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/)
include_directories(include/)

# enable code coverage
find_package(codecov)

add_executable(jcc_test ${TEST_SOURCES} ${TEST_HEADERS} src/test_printer.cpp)
add_coverage(jcc_test)

#
# Dependencies.
#
#-------------------------------------------------------------------------------

# jcc
include_directories(${CMAKE_SOURCE_DIR}/include)
target_link_libraries(jcc_test ${CMAKE_SOURCE_DIR}/build/libjcc.so)

#-------------------------------------------------------------------------------

add_test(main ${CMAKE_SOURCE_DIR}/build/jcc_test)
