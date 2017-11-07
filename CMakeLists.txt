cmake_minimum_required(VERSION 3.9)
project(cmkizer)

set(HEADERS)

set(SOURCES
    src/main.cpp
)

add_executable(cmkizer ${SOURCES} ${HEADERS})
target_compile_options(cmkizer PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17>)