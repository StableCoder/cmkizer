/*
 *  MIT License
 *
 *  Copyright (c) 2017 George Cave
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#ifndef FILE_PARSER_HPP
#define FILE_PARSER_HPP

// cmkizer
#include "type_defs.hpp"

// C++
#include <string>
#include <string_view>
#include <tuple>

/// \brief Parses a project file, typically a .sln file.
/// \param projectPath The path to the project file to parse.
/// \return A boolean representing th success, and ProjectData for a successful
/// parse.
std::tuple<bool, ProjectData> parseProject(std::string_view projectPath);

/// \brief Parses a target file, typically a .vcproj or vcxproj file.
/// \param projectPath The path to the target file to parse.
/// \return A boolean representing th success, and TargetData for a successful
/// parse.
std::tuple<bool, TargetData> parseTarget(std::string_view targetPath);

// std::tuple<bool, SetupData> parseSetup(const std::string& setupPath);

#endif // FILE_PARSER_HPP
