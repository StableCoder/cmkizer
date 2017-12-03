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