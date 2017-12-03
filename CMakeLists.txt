cmake_minimum_required(VERSION 3.9)
project(cmkizer)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C++17 for MSVC before CMake 3.10
if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

# Clang Tools
file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.[ch]pp
    ${PROJECT_SOURCE_DIR}/src/*.[ch]
)

if(NOT CLANG_FORMAT)
    find_program(CLANG_FORMAT "clang-format")
endif()
if(CLANG_FORMAT)
    add_custom_target(
        ${PROJECT_NAME}_format
        COMMAND clang-format
        -i
        -style=file
        ${ALL_CXX_SOURCE_FILES}
    )

    if(NOT TARGET format)
        add_custom_target(format)
    endif()

    add_dependencies(format ${PROJECT_NAME}_format)
endif()

if(NOT CLANG_TIDY)
    find_program(CLANG_TIDY "clang-tidy")
endif()
if(CLANG_TIDY)
    add_custom_target(
        ${PROJECT_NAME}_tidy
        COMMAND clang-tidy
        ${ALL_CXX_SOURCE_FILES}
        -checks=*,-cert-err58-cpp,-readability-else-after-return,-readability-named-parameter,-llvm-header-guard,-readability-avoid-const-params-in-decls,-readability-implicit-bool-cast,-google-build-using-namespace,-cppcoreguidelines-pro-bounds-constant-array-index,-google-readability-namespace-comments,-llvm-namespace-comment,-google-explicit-constructor,-readability-redundant-member-init,-misc-unused-parameters,-google-runtime-int,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-modernize-use-auto,-hicpp-explicit-conversions
        --
        -std=c++17
        -I${CMAKE_CURRENT_SOURCE_DIR}/ext
    )

    if(NOT TARGET tidy)
        add_custom_target(tidy)
    endif()

    add_dependencies(tidy ${PROJECT_NAME}_tidy)
endif()

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

add_executable(cmkizer ${SOURCES})
if(MSVC)
    target_link_libraries(cmkizer libxml2)
else()
    target_link_libraries(cmkizer xml2)
endif()