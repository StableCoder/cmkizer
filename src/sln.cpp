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

#include "sln.hpp"

// cmkizer
#include "file_parser.hpp"

// C++
#include <algorithm>
#include <cstring>
#include <fstream>

std::tuple<bool, ProjectData> slnProjectParse(const std::string &projectPath) {
  std::ifstream inFile(projectPath, std::ios::in);
  if (!inFile) {
    return std::make_tuple(false, ProjectData());
  }

  ProjectData data;
  const auto lastSlash(
      std::min(projectPath.find_last_of('\\'), projectPath.find_last_of('/')));
  std::string rootPath;
  if (lastSlash != std::string::npos) {
    rootPath = projectPath.substr(0, lastSlash + 1);
    std::replace(rootPath.begin(), rootPath.end(), '\\', '/');
    data.name = projectPath.substr(
        rootPath.size(), projectPath.find_last_of('.') - rootPath.size());
  } else {
    // If no path, then use local dir as prefix.
    rootPath = "./";
    data.name = projectPath.substr(0, projectPath.find_last_of('.'));
  }
  data.path = projectPath;

  bool dependencyMode = false;
  TargetData *activeTarget = nullptr;

  while (!inFile.eof()) {
    std::string line;
    std::getline(inFile, line);

    if (line.find("EndProject") != std::string::npos) {
      activeTarget = nullptr;
      dependencyMode = false;
    } else if (dependencyMode) {
      if (line.find("EndProjectSection") != std::string::npos) {
        dependencyMode = false;
      } else {
        const auto start = line.find_first_of('{');
        const auto end = line.find_first_of('}');

        if (start != std::string::npos && end != std::string::npos) {
          activeTarget->dependencies.emplace_back(
              line.substr(start, (end + 1) - start));
        }
      }
    } else if (line.find("ProjectSection(ProjectDependencies)") !=
                   std::string::npos &&
               activeTarget != nullptr) {
      // Enter dependency mode, and add dependencies.
      dependencyMode = true;
    } else if (line.find("Project(\"{") != std::string::npos) {
      // A target definition.
      auto start = line.find('"', 50);
      auto end = line.find('"', ++start);
      std::string name = line.substr(start, end - start);

      if (name == "Build" && data.targets.empty()) {
        continue;
      }

      start = line.find('"', ++end);
      end = line.find('"', ++start);
      std::string relativePath = line.substr(start, end - start);
      std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
      std::string fullPath = rootPath + relativePath;

      auto[read, target] = parseTarget(fullPath);

      if (!read) {
        printf("Error: Could not parse project file - %s\n",
               relativePath.data());
        continue;
      }

      target.name = name;
      target.relativePath = relativePath;

      start = line.find('"', ++end);
      end = line.find('"', ++start);
      target.displayName = line.substr(start, end - start);

      data.targets.emplace_back(target);

      activeTarget = &*(data.targets.end() - 1);
    }
  }

  return std::make_tuple(true, data);
}