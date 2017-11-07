cmake_minimum_required(VERSION 3.9)
project(cmkizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C++17 for MSVC before CMake 3.10
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

set(HEADERS
    src/generators.hpp
    src/file_parser.hpp
    src/util.hpp
    src/dsp.hpp
    src/dsw.hpp
)

set(SOURCES
    src/generators.cpp
    src/file_parser.cpp
    src/util.cpp
    src/dsp.cpp
    src/dsw.cpp
    src/main.cpp
)

add_executable(cmkizer ${SOURCES} ${HEADERS})