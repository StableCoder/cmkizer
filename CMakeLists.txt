cmake_minimum_required(VERSION 3.9)
project(cmkizer CXX)

include(cmake/c++-standards.cmake)
include(cmake/clang-tools.cmake)
include(cmake/code-coverage.cmake)
include(cmake/sanitizer-builds.cmake)

file(GLOB_RECURSE ALL_CODE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.[ch]pp
)

_Cxx17()
_ClangFormat(cmkizer ${ALL_CODE_FILES})
_ClangTidy(cmkizer ${ALL_CODE_FILES})

# libxml2
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

_AddExecutable(cmkizer ${SOURCES})
target_include_directories(cmkizer PRIVATE ${LIBXML2_INCLUDE_DIR})
if(MSVC)
    target_link_libraries(cmkizer libxml2)
else()
    target_link_libraries(cmkizer xml2)
endif()