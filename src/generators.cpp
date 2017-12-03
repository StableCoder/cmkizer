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

#include "generators.hpp"

// cmkizer
#include "util.hpp"

// C++
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
      for (auto &otherTarget : data.targets) {
        if (otherTarget.displayName == dependency) {
          for (auto &config : target.configs) {
            config.linkLibraries.emplace_back(otherTarget.name);
          }
          break;
        }
      }
    }
  }

  // Collapse target configs that are the same
  for (auto &target : data.targets) {
    for (auto config = target.configs.begin(); config != target.configs.end();
         ++config) {
      for (auto otherConfig = config + 1; otherConfig < target.configs.end();
           ++otherConfig) {
        if (config->name == otherConfig->name &&
            config->definitions.size() == otherConfig->definitions.size() &&
            config->includes.size() == otherConfig->includes.size() &&
            config->linkLibraries.size() == otherConfig->linkLibraries.size()) {
          bool same = true;

          // Compare includes
          for (auto &it : config->includes) {
            bool found = false;
            for (auto &otherIt : otherConfig->includes) {
              if (it == otherIt) {
                found = true;
                break;
              }
            }
            if (!found) {
              same = false;
            }
          }

          // Compare definitions
          for (auto &it : config->definitions) {
            bool found = false;
            for (auto &otherIt : otherConfig->definitions) {
              if (it == otherIt) {
                found = true;
                break;
              }
            }
            if (!found) {
              same = false;
            }
          }

          // Compare libraries
          for (auto &it : config->linkLibraries) {
            bool found = false;
            for (auto &otherIt : otherConfig->linkLibraries) {
              if (it == otherIt) {
                found = true;
                break;
              }
            }
            if (!found) {
              same = false;
            }
          }

          if (same) {
            target.configs.erase(otherConfig);
          }
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

void generateCMakeProject(const ProjectData &data) {
  // Open own file.
  std::string outFilePath;
  auto lastSlash =
      std::min(data.path.find_last_of('/'), data.path.find_last_of('\\'));
  if (lastSlash == std::string::npos) {
    outFilePath = "./";
  } else {
    lastSlash++;
    outFilePath = data.path.substr(0, lastSlash);
  }
  outFilePath += cCmakeFilename;
  FILE *pOut = fopen(outFilePath.c_str(), "w");
  if (pOut == nullptr) {
    printf("cmkizer: Failed to open file to send CMake output to - %s",
           outFilePath.c_str());
    return;
  }
  fprintf(pOut, "cmake_minimum_required( VERSION %s )\n",
          gGlobalSettings.cmakeVersion.data());
  fprintf(pOut, "project ( %s )\n\n", data.name.data());

  for (auto &target : data.targets) {
    if (target.relativePath.find('/') == std::string::npos) {
      // Put it in the same file, since it's in the same folder.
      fprintf(pOut, "\n\n# %s Target\n", target.name.data());
      generateCMakeTarget(target, false, pOut);
    } else {
      generateCMakeTarget(target, false, nullptr);
      if (target.relativePath.find('"') != std::string::npos) {
        // If the path has a space in it, surround with quotes.
        fprintf(
            pOut, "add_subdirectory( \"%s\" )\n",
            target.relativePath.substr(0, target.relativePath.find_last_of('/'))
                .data());
      } else {
        fprintf(
            pOut, "add_subdirectory( %s )\n",
            target.relativePath.substr(0, target.relativePath.find_last_of('/'))
                .data());
      }
    }
  }

  fclose(pOut);
}

void generateCMakeTarget(const TargetData &data, bool standalone,
                         FILE *pOutfile) {
  FILE *pOut = pOutfile;
  if (pOut == nullptr) {
    // Open own file.
    std::string outFilePath;
    auto lastSlash = std::min(data.fullPath.find_last_of('/'),
                              data.fullPath.find_last_of('\\'));
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
  if (standalone) {
    fprintf(pOut, "project( %s )\n", data.name.data());
  }
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
      fprintf(pOut,
              "    if (MSVC)\n        target_compile_definitions( %s PUBLIC",
              it.name.data());
      for (auto &def : it.definitions) {
        fprintf(pOut, " /D%s", def.data());
      }
      fprintf(pOut, " )\n");
      // For Non-MSVC
      fprintf(pOut,
              "    else ()\n        target_compile_definitions( %s PUBLIC",
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

    // targets
    fprintf(pOut, "\n    install(\n");
    fprintf(pOut, "            TARGETS %s\n", it.name.data());
    fprintf(pOut, "            RUNTIME DESTINATION bin\n");
    fprintf(pOut, "            COMPONENT binaries\n");
    fprintf(pOut, "            LIBRARY DESTINATION lib\n");
    fprintf(pOut, "            ARCHIVE DESTINATION lib\n");
    fprintf(pOut, "            COMPONENT libraries\n    )\n");

    fprintf(pOut, "endif()\n");
  }

  // Install
  fprintf(pOut, "\n# Install\n");
  // h
  fprintf(pOut, "install(\n");
  fprintf(pOut, "        DIRECTORY ./\n");
  fprintf(pOut, "        DESTINATION %s\n", gGlobalSettings.includePath.data());
  fprintf(pOut, "        COMPONENT headers\n");
  fprintf(pOut, "        FILES MATCHING PATTERN \"*.h\"\n)\n");
  // hpp
  fprintf(pOut, "\ninstall(\n");
  fprintf(pOut, "        DIRECTORY ./\n");
  fprintf(pOut, "        DESTINATION %s\n", gGlobalSettings.includePath.data());
  fprintf(pOut, "        COMPONENT headers\n");
  fprintf(pOut, "        FILES MATCHING PATTERN \"*.hpp\"\n)\n");

  if (pOutfile == nullptr) {
    fclose(pOut);
  }
}