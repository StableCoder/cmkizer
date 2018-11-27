/*
 *  MIT License
 *
 *  Copyright (c) 2018 George Cave <gcave@stablecoder.ca>
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

#ifndef UTIL_HPP
#define UTIL_HPP

// cmizer
#include "type_defs.hpp"

// C++
#include <cstdio>
#include <string>
#include <vector>

/// Determines, using the file's extension whether the file is a header, source,
/// or resource file.
///
/// Upon discovery of the type, the file will be added to the appropriate list
/// in the filter group, and will also set the language in the TargetData.
/// \param fileName The filename to determine what kind of file it is.
/// \param data The TargetData struct to fill with the language of the file.
/// \param group The FilterGroup the file will belong to.
void determineLanguage(std::string fileName, TargetData &data, FilterGroup &group);

/// Parses a target's output file to determine if it is a library or an
/// executable.
/// \param outputName The name of the output to determine if it is a library.
/// \return True if a library, false otherwise.
bool checkIsLibrary(const std::string_view outputName);

/// Parses a string using common delimiters to split it into an array of strings
/// instead.
/// \param items The straing to split and return.
/// \return A vector of the split strings.
std::vector<std::string> parseItems(const std::string_view items);

/// Removes typical default definitions from MSVS
/// \param definitionList The list of definitions to purify.
void removeDefaultDefinitions(std::vector<std::string> &definitionList);

/// Removes typical default include paths from MSVS
/// \param includeList The list of includes to purify.
void removeDefaultIncludes(std::vector<std::string> &includeList);

std::vector<std::string> parseDefinitions(std::string_view definitions) noexcept;

#endif // UTIL_HPP
