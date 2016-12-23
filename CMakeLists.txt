#
# jcc's CMakeLists file.
#

cmake_minimum_required(VERSION 2.8)
project(jcc)

file(GLOB_RECURSE JCC_HEADERS ${CMAKE_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE JCC_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/)
include_directories(include/)
add_library(jcc SHARED ${JCC_SOURCES} ${JCC_HEADERS} include/linker/translators/elf64/object_file.hpp src/linker/translators/elf64/object_file.cpp include/linker/translators/elf64/section.hpp src/linker/translators/elf64/section.cpp include/common/binary.hpp include/linker/translators/elf64/segment.hpp src/linker/translators/elf64/segment.cpp src/assembler/relocation.cpp src/linker/translators/elf64/elf64.cpp include/linker/linker.hpp src/linker/linker.cpp)

add_subdirectory(test)

#
# Dependencies.
#
#-------------------------------------------------------------------------------

# Boost
find_package(Boost COMPONENTS system program_options filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(jcc ${Boost_LIBRARIES})

#-------------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -g -Wall")

