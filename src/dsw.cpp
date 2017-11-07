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

#include "dsw.hpp"

// cmkizer
#include "file_parser.hpp"

// C++
#include <algorithm>
#include <cstring>
#include <fstream>

std::tuple<bool, ProjectData> dswProjectParse(const std::string &projectPath) {
  ProjectData data;
  std::ifstream inFile(projectPath, std::ios::in);
  if (!inFile) {
    return std::make_tuple(false, data);
  }

  TargetData *currentTarget = nullptr;
  std::string rootPath;
  const auto lastSlash =
      std::min(projectPath.find_last_of('/'), projectPath.find_last_of('\\'));
  if (lastSlash != std::string::npos) {
    rootPath = projectPath.substr(0, lastSlash + 1);
    std::replace(rootPath.begin(), rootPath.end(), '\\', '/');
    data.name = projectPath.substr(
        rootPath.size(), projectPath.find_last_of('.') - rootPath.size());
  } else {
    rootPath = "./";
    data.name = projectPath.substr(0, projectPath.find_last_of('.'));
  }
  data.path = projectPath;

  while (!inFile.eof()) {
    std::string line;
    std::getline(inFile, line);

    if (line.find("Project: \"") != std::string::npos) {
      line.erase(0, strlen("Project: \""));
      std::string targetName = line.substr(0, line.find('\"'));
      line.erase(0, targetName.size() + 3);
      std::string relativePath(line.substr(0, line.find('\"')));
      std::replace(relativePath.begin(), relativePath.end(), '\\', '/');
      if (relativePath.find_first_of("./") == 0) {
        relativePath.erase(0, 2);
      }
      std::string fullPath(rootPath + relativePath);
      std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
      auto[read, targetData] = parseTarget(fullPath);

      if (read) {
        targetData.name = targetName;
        targetData.displayName = targetName;
        targetData.fullPath = fullPath;
        targetData.relativePath = relativePath;
        data.targets.emplace_back(targetData);
        currentTarget = &*(data.targets.end() - 1);
      } else {
        currentTarget = nullptr;
        printf("Error: Could not parse project file - %s\n",
               relativePath.data());
        continue;
      }
    } else if (line.find("Project_Dep_Name ") != std::string::npos) {
      line.erase(0,
                 line.find("Project_Dep_Name ") + strlen("Project_Dep_Name "));
      if (currentTarget != nullptr) {
        currentTarget->dependencies.emplace_back(line);
      }
    }
  }

  return std::make_tuple(true, data);
}