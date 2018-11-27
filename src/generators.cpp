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

#include "generators.hpp"

#include "util.hpp"

#include <algorithm>
#include <cctype>
#include <tuple>

ProjectData projectPreprocessing(ProjectData data) {
    if (data.targets.empty()) {
        // Do nothing, there are no targets.
        return data;
    }

    // Remove typical OS-specific flags that are given to targets by default.
    for (auto &target : data.targets) {
        for (auto &config : target.configs) {
            removeDefaultDefinitions(config.definitions);
            removeDefaultIncludes(config.includes);
        }
    }

    // Get the best target names
    for (auto &target : data.targets) {
        std::vector<std::tuple<std::string, int>> configCount;
        for (auto &config : target.configs) {
            bool counted = false;
            for (auto &count : configCount) {
                if (std::get<0>(count) == config.name) {
                    std::get<1>(count)++;
                    counted = true;
                    break;
                }
            }
            if (!counted) {
                configCount.emplace_back(config.name, 1);
            }
        }

        std::vector<std::tuple<std::string, int>> topCounted;
        for (auto &counts : configCount) {
            if (topCounted.empty()) {
                topCounted.emplace_back(counts);
            } else {
                if (std::get<1>(counts) > std::get<1>(topCounted[0])) {
                    topCounted.clear();
                    topCounted.emplace_back(counts);
                } else if (std::get<1>(counts) == std::get<1>(topCounted[0])) {
                    topCounted.emplace_back(counts);
                }
            }
        }
        if (topCounted.size() == 1) {
            target.name = std::get<0>(topCounted[0]);
        } else {
            // Something else
            target.name = std::get<0>(topCounted[0]);
        }
    }

    // Link project dependencies
    for (auto &target : data.targets) {
        for (auto &dependency : target.dependencies) {
            std::string comp1 = dependency;
            std::transform(comp1.begin(), comp1.end(), comp1.begin(), ::toupper);

            for (auto &otherTarget : data.targets) {
                std::string comp2 = otherTarget.displayName;
                std::transform(comp2.begin(), comp2.end(), comp2.begin(), ::toupper);

                if (comp1 == comp2) {
                    for (auto &config : target.configs) {
                        config.linkLibraries.emplace_back(otherTarget.name);
                    }
                    break;
                }
            }
        }
    }

    // Convert include paths to correct slash format
    for (auto &target : data.targets) {
        for (auto &config : target.configs) {
            for (auto &include : config.includes) {
                std::replace(include.begin(), include.end(), '\\', '/');
            }
        }
    }

    // Check for QT items
    if (gGlobalSettings.qtVersion != 0) {
        for (auto &target : data.targets) {
            for (auto &filter : target.groups) {
                // Check file for QT MOC/UIC/RCC options.

                auto qtFile = filter.sourceFiles.begin();
                while (qtFile != filter.sourceFiles.end()) {
                    auto start = qtFile->find_last_of('/');
                    if (start == std::string::npos) {
                        start = 0;
                    } else {
                        ++start;
                    }
                    auto end = qtFile->find_last_of('.');
                    std::string_view fileName(qtFile->data() + start, end - start);
                    std::string_view ext = qtFile->data() + end;

                    if (fileName.substr(0, 4) == "moc_" || ext == ".moc" ||
                        fileName.substr(0, 4) == "qrc_" || ext == ".qrc" ||
                        fileName.substr(0, 3) == "ui_" || ext == ".ui") {
                        target.useQt = true;
                    }

                    ++qtFile;
                }
            }
        }
    }

    return data;
}

void generateCMakeProject(const ProjectData &projectData) {
    // Open own file.
    std::string outFilePath;
    auto lastSlash =
        std::min(projectData.path.find_last_of('/'), projectData.path.find_last_of('\\'));
    if (lastSlash == std::string::npos) {
        outFilePath = "./";
    } else {
        lastSlash++;
        outFilePath = projectData.path.substr(0, lastSlash);
    }
    outFilePath += cCmakeFilename;
    FILE *pOut = fopen(outFilePath.c_str(), "w");
    if (pOut == nullptr) {
        printf("cmkizer: Failed to open file to send CMake output to - %s", outFilePath.c_str());
        return;
    }
    fprintf(pOut, "cmake_minimum_required( VERSION %s )\n", gGlobalSettings.cmakeVersion.data());
    fprintf(pOut, "project ( %s )\n\n", projectData.name.data());

    for (auto &target : projectData.targets) {
        if (target.relativePath.find('/') == std::string::npos) {
            // Put it in the same file, since it's in the same folder.
            fprintf(pOut, "\n\n# %s Target\n", target.name.data());
            generateCMakeTarget(target, pOut);
        } else {
            generateCMakeTarget(target, nullptr);
            if (target.relativePath.find('"') != std::string::npos) {
                // If the path has a space in it, surround with quotes.
                fprintf(
                    pOut, "add_subdirectory( \"%s\" )\n",
                    target.relativePath.substr(0, target.relativePath.find_last_of('/')).data());
            } else {
                fprintf(
                    pOut, "add_subdirectory( %s )\n",
                    target.relativePath.substr(0, target.relativePath.find_last_of('/')).data());
            }
        }
    }

    fclose(pOut);
}

void generateCMakeTarget(const TargetData &data, FILE *pOutfile) {
    FILE *pOut = pOutfile;
    if (pOut == nullptr) {
        // Open own file.
        std::string outFilePath;
        auto lastSlash =
            std::min(data.fullPath.find_last_of('/'), data.fullPath.find_last_of('\\'));
        if (lastSlash == std::string::npos) {
            outFilePath = "./";
        } else {
            lastSlash++;
            outFilePath = data.fullPath.substr(0, lastSlash);
        }
        outFilePath += cCmakeFilename;
        pOut = fopen(outFilePath.c_str(), "w");
        if (pOut == nullptr) {
            printf("cmkizer: Failed to open file to send CMake output to - %s",
                   outFilePath.c_str());
            return;
        }
        fprintf(pOut, "cmake_minimum_required( VERSION %s )\n",
                gGlobalSettings.cmakeVersion.data());
    }

    // Project Name
    fprintf(pOut, "project( %s )\n", data.name.data());

    // Languages
    fprintf(pOut, "enable_language( ");
    if (data.enableC) {
        fprintf(pOut, "C ");
    }
    if (data.enableCXX) {
        fprintf(pOut, "CXX ");
    }
    if (data.enableFortran) {
        fprintf(pOut, "Fortran ");
    }
    fprintf(pOut, ")\n");

    // File Sets
    fprintf(pOut, "\n# File Sets");
    for (auto &group : data.groups) {
        if (!group.headerFiles.empty()) {
            std::string temp = group.name + " headers";
            std::replace(temp.begin(), temp.end(), ' ', '_');
            std::transform(temp.begin(), temp.end(), temp.begin(),
                           [](unsigned char c) { return std::toupper(c); });
            fprintf(pOut, "\nset(\n    %s\n", temp.data());
            for (auto &it : group.headerFiles) {
                fprintf(pOut, "    %s\n", it.data());
            }
            fprintf(pOut, ")\n");
        }
        if (!group.sourceFiles.empty()) {
            std::string temp = group.name + " sources";
            std::replace(temp.begin(), temp.end(), ' ', '_');
            std::transform(temp.begin(), temp.end(), temp.begin(),
                           [](unsigned char c) { return std::toupper(c); });
            fprintf(pOut, "\nset(\n    %s\n", temp.data());
            for (auto &it : group.sourceFiles) {
                fprintf(pOut, "    %s\n", it.data());
            }
            fprintf(pOut, ")\n");
        }
        if (!group.resourceFiles.empty()) {
            std::string temp = group.name + " resources";
            std::replace(temp.begin(), temp.end(), ' ', '_');
            std::transform(temp.begin(), temp.end(), temp.begin(),
                           [](unsigned char c) { return std::toupper(c); });
            fprintf(pOut, "\nset(\n    %s\n", temp.data());
            for (auto &it : group.resourceFiles) {
                fprintf(pOut, "    %s\n", it.data());
            }
            fprintf(pOut, ")\n");
        }
    }

    // QT
    if (data.useQt != 0) {
        fprintf(pOut, "\n# Qt\n");
        fprintf(pOut, "find_package(Qt%d REQUIRED)\n", gGlobalSettings.qtVersion);
        fprintf(pOut, "set(CMAKE_AUTOMOC ON)\n");
        fprintf(pOut, "set(CMAKE_AUTOUIC ON)\n");
        fprintf(pOut, "set(CMAKE_AUTORCC ON)\n");
        fprintf(pOut, "set(CMAKE_INCLUDE_CURRENT_DIR ON)\n");
    }

    // Configurations
    for (auto &it : data.configs) {
        fprintf(pOut, "\n# Configuration - %s\n", it.description.data());
        fprintf(pOut, "if()\n");
        // MFC
        if (data.useMFC == 6) {
            fprintf(pOut, "    set(CMAKE_MFC_FLAG 2)\n");
        } else if (data.useMFC != 0) {
            fprintf(pOut, "    set(CMAKE_MFC_FLAG 1)\n");
        }

        // Target
        if (data.isLibrary) {
            fprintf(pOut, "    add_library( %s", it.name.data());
        } else {
            fprintf(pOut, "    add_executable( %s", it.name.data());
        }
        for (auto &group : data.groups) {
            std::string temp(group.name);
            std::replace(temp.begin(), temp.end(), ' ', '_');
            std::transform(temp.begin(), temp.end(), temp.begin(),
                           [](unsigned char c) { return std::toupper(c); });
            if (!group.sourceFiles.empty()) {
                fprintf(pOut, " ${%s_SOURCES}", temp.data());
            }
            if (!group.headerFiles.empty()) {
                fprintf(pOut, " ${%s_HEADERS}", temp.data());
            }
        }
        fprintf(pOut, " )\n");

        // Definitions
        if (!it.definitions.empty()) {
            // For MSVC
            fprintf(pOut, "    if (MSVC)\n        target_compile_definitions( %s PUBLIC",
                    it.name.data());
            for (auto &def : it.definitions) {
                fprintf(pOut, " /D%s", def.data());
            }
            fprintf(pOut, " )\n");
            // For Non-MSVC
            fprintf(pOut, "    else ()\n        target_compile_definitions( %s PUBLIC",
                    it.name.data());
            for (auto &def : it.definitions) {
                fprintf(pOut, " -D%s", def.data());
            }
            fprintf(pOut, " )\n    endif ()\n");
        }

        // Includes
        if (!it.includes.empty()) {
            fprintf(pOut, "    target_include_directories( %s", it.name.data());
            for (auto &inc : it.includes) {
                fprintf(pOut, " %s", inc.data());
            }
            fprintf(pOut, " )\n");
        }

        // Libraries
        if (!it.linkLibraries.empty()) {
            fprintf(pOut, "    target_link_libraries( %s", it.name.data());
            for (auto &lib : it.linkLibraries) {
                fprintf(pOut, " %s", lib.data());
            }
            fprintf(pOut, " )\n");
        }

        fprintf(pOut, "endif()\n");
    }

    if (pOutfile == nullptr) {
        fclose(pOut);
    }
}