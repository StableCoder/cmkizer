cmake_minimum_required(VERSION 3.9)
project(cmkizer CXX)

include(cmake/c++-standards.cmake)
include(cmake/code-coverage.cmake)
include(cmake/sanitizers.cmake)
include(cmake/tools.cmake)

# Compiler Options
cxx_17()

# External Libraries
find_package(LibXml2 REQUIRED)

# Executable
set(SOURCES
    src/generators.cpp
    src/file_parser.cpp
    src/util.cpp
    src/dsp.cpp
    src/dsw.cpp
    src/proj.cpp
    src/xproj.cpp
    src/sln.cpp
    src/main.cpp
)

add_executable(cmkizer ${SOURCES})
target_include_directories(cmkizer PRIVATE ${LIBXML2_INCLUDE_DIR})
if(MSVC)
    target_link_libraries(cmkizer libxml2)
else()
    target_link_libraries(cmkizer xml2)
endif()