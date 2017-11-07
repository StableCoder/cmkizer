cmake_minimum_required(VERSION 3.9)
project(cmkizer)

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
target_compile_options(cmkizer PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>)