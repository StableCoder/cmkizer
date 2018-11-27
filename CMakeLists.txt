cmake_minimum_required(VERSION 3.9)
project(cmkizer CXX)

include(cmake/c++-standards.cmake)
include(cmake/code-coverage.cmake)
include(cmake/default-output-dirs.cmake)
include(cmake/sanitizers.cmake)
include(cmake/tools.cmake)

# Compiler Options
cxx_17()

# External Libraries
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
find_package(libxml2)

# Executable
add_executable(cmkizer 
    src/generators.cpp
    src/file_parser.cpp
    src/util.cpp
#    src/dsp.cpp
#    src/dsw.cpp
#    src/proj.cpp
    src/xproj.cpp
    src/vfproj.cpp
    src/sln.cpp
    src/main.cpp
)
target_link_libraries(cmkizer PRIVATE stdc++fs)

if(TARGET libxml2::libxml2)
    target_link_libraries(cmkizer PRIVATE libxml2::libxml2)
else()
    find_package(LibXml2 REQUIRED)
    target_include_directories(cmkizer PRIVATE ${LIBXML2_INCLUDE_DIRS})
    target_link_libraries(cmkizer PRIVATE ${LIBXML2_LIBRARIES})
endif()