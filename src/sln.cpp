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
    const auto lastSlash(std::min(projectPath.find_last_of('\\'), projectPath.find_last_of('/')));
    std::string rootPath;
    if (lastSlash != std::string::npos) {
        rootPath = projectPath.substr(0, lastSlash + 1);
        std::replace(rootPath.begin(), rootPath.end(), '\\', '/');
        data.name =
            projectPath.substr(rootPath.size(), projectPath.find_last_of('.') - rootPath.size());
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
                    activeTarget->dependencies.emplace_back(line.substr(start, (end + 1) - start));
                }
            }
        } else if (line.find("ProjectSection(ProjectDependencies)") != std::string::npos &&
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
                printf("Error: Could not parse project file - %s\n", relativePath.data());
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