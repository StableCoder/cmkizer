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

#ifndef TYPE_DEFS_HPP
#define TYPE_DEFS_HPP

// C++
#include <string>
#include <vector>

/// A grouping of files together, separated as source, header and resource
/// files.
struct FilterGroup {
  std::string name;
  std::vector<std::string> sourceFiles;
  std::vector<std::string> headerFiles;
  std::vector<std::string> resourceFiles;
};

/// A particular configuration of a target, including lists for the particular
/// includes, libraries and definitions.
struct TargetConfig {
  std::string name;
  std::string description;
  std::vector<std::string> definitions;
  std::vector<std::string> includes;
  std::vector<std::string> linkLibraries;
};

/// A full target's information, including the file's location, the configs,
/// files, and languages used.
struct TargetData {
  std::string name;
  std::string displayName;
  std::string fullPath;
  std::string relativePath;
  std::vector<TargetConfig> configs;
  std::vector<FilterGroup> groups;
  std::vector<std::string> dependencies;
  bool enableC = false;
  bool enableCXX = false;
  bool enableFortran = false;
  bool isLibrary = false;
  int useMFC = 0;
  bool useQt = false;
};

/// A full project's information.
struct ProjectData {
  std::string name;
  std::string path;
  std::vector<TargetData> targets;
};

struct GlobalSettings {
  int qtVersion = 0;
  std::string includePath = "include/";
  std::string cmakeVersion = "3.9";
  int cpackType = 0;
};

constexpr const char *cCmakeFilename("CMakeLists.txt");

#endif // TYPE_DEFS_HPP
