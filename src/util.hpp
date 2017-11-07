/*
BSD 2-Clause License

Copyright (c) 2017, Stable Tec
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UTIL_HPP
#define UTIL_HPP

// cmizer
#include "type_defs.hpp"

// C++
#include <cstdio>
#include <string>
#include <vector>

static GlobalSettings gGlobalSettings;

/// Determines, using the file's extension whether the file is a header, source,
/// or resource file.
///
/// Upon discovery of the type, the file will be added to the appropriate list
/// in the filter group, and will also set the language in the TargetData.
/// \param fileName The filename to determine what kind of file it is.
/// \param data The TargetData struct to fill with the language of the file.
/// \param group The FilterGroup the file will belong to.
void determineLanguage(const std::string_view fileName, TargetData &data,
                       FilterGroup &group);

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

#endif // UTIL_HPP
