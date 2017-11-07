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

#ifndef FILE_PARSER_HPP
#define FILE_PARSER_HPP

// cmkizer
#include "type_defs.hpp"

// C++
#include <string>
#include <tuple>

/// \brief Parses a project file, typically a .sln file.
/// \param projectPath The path to the project file to parse.
/// \return A boolean representing th success, and ProjectData for a successful
/// parse.
std::tuple<bool, ProjectData> parseProject(const std::string &projectPath);

/// \brief Parses a target file, typically a .vcproj or vcxproj file.
/// \param projectPath The path to the target file to parse.
/// \return A boolean representing th success, and TargetData for a successful
/// parse.
std::tuple<bool, TargetData> parseTarget(const std::string &targetPath);

// std::tuple<bool, SetupData> parseSetup(const std::string& setupPath);

#endif // FILE_PARSER_HPP
