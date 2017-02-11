#
# jcc's CMakeLists file.
#

cmake_minimum_required(VERSION 2.8)
project(jcc)

file(GLOB_RECURSE JCC_HEADERS ${CMAKE_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE JCC_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -g -Wall")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/externals/CMake-codecov/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/)
include_directories(include/)
include_directories(${CMAKE_SOURCE_DIR}/externals/Catch/include/)
enable_testing()

# enable code coverage
find_package(codecov)

add_library(jcc SHARED ${JCC_SOURCES} ${JCC_HEADERS} include/linker/translators/elf64/object_file.hpp src/linker/translators/elf64/object_file.cpp include/linker/translators/elf64/section.hpp src/linker/translators/elf64/section.cpp include/common/binary.hpp include/linker/translators/elf64/segment.hpp src/linker/translators/elf64/segment.cpp src/assembler/relocation.cpp src/linker/translators/elf64/elf64.cpp include/linker/linker.hpp src/linker/linker.cpp include/jtac/jtac.hpp include/jtac/assembler.hpp src/jtac/assembler.cpp include/jtac/control_flow.hpp src/jtac/control_flow.cpp include/jtac/ssa.hpp src/jtac/ssa.cpp include/jtac/printer.hpp src/jtac/printer.cpp src/jtac/jtac.cpp include/jtac/data_flow.hpp src/jtac/data_flow.cpp include/jtac/allocation/allocator.hpp include/jtac/allocation/basic/basic.hpp src/jtac/allocation/basic/basic.cpp include/jtac/allocation/basic/undirected_graph.hpp src/jtac/allocation/basic/undirected_graph.cpp include/jtac/program.hpp src/jtac/program.cpp include/jtac/parse/lexer.hpp include/jtac/parse/token.hpp src/jtac/parse/token.cpp src/jtac/parse/lexer.cpp include/jtac/parse/parser.hpp src/jtac/parse/parser.cpp tools/test/main.cpp include/jtac/name_map.hpp include/jtac/translate/x86_64/x86_64_translator.hpp include/jtac/translate/x86_64/procedure.hpp src/jtac/translate/x86_64/x86_64_translator.cpp src/jtac/allocation/allocator.cpp)
add_coverage(jcc)

add_subdirectory(test)
add_subdirectory(tools)

#
# Dependencies.
#
#-------------------------------------------------------------------------------



#-------------------------------------------------------------------------------

# evaluate coverage
coverage_evaluate()
