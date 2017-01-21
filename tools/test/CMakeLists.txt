
cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/tools/test/*.hpp)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/tools/test/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -O0 -g")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/)
include_directories(include/)

add_executable(test_app ${SOURCES} ${HEADERS})

#
# Dependencies.
#
#-------------------------------------------------------------------------------

# jcc
include_directories(${CMAKE_SOURCE_DIR}/include)
target_link_libraries(test_app ${CMAKE_SOURCE_DIR}/build/libjcc.so)

#-------------------------------------------------------------------------------
