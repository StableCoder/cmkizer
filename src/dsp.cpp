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

#include "dsp.hpp"

// cmkizer
#include "util.hpp"

// C++
#include <algorithm>
#include <cstring>
#include <fstream>

std::tuple<bool, TargetData> dspTargetParse(const std::string &filePath) {
  std::ifstream inFile(filePath, std::ios::in);
  if (!inFile) {
    return std::make_tuple(false, TargetData());
  }

  TargetData data;
  FilterGroup *activeFilter = nullptr;
  TargetConfig *activeConfig = nullptr;

  while (!inFile.eof()) {
    std::string line;
    std::getline(inFile, line);

    // File Filter
    if (line.find("# Begin Group \"") != std::string::npos) {
      line.erase(0, strlen("# Begin Group \""));
      FilterGroup newGroup;
      newGroup.name = line.substr(0, line.find('\"'));
      data.groups.push_back(newGroup);
      activeFilter = &*(data.groups.end() - 1);
    }

    // Files
    if (line.find("SOURCE=") != std::string::npos) {
      line.erase(0, strlen("SOURCE="));
      // Unixify paths
      std::replace(line.begin(), line.end(), '\\', '/');
      line.erase(std::remove(line.begin(), line.end(), '\"'), line.end());
      if (line.substr(0, 2) == "./") {
        line.erase(0, 2);
      }

      determineLanguage(line, data, *activeFilter);
    }

    // Configurations
    if (line.find("!IF  \"$(CFG)\" == \"") != std::string::npos ||
        line.find("!ELSEIF  \"$(CFG)\" == \"") != std::string::npos) {
      if (line.find("!IF  \"$(CFG)\" == \"") != std::string::npos) {
        line.erase(0, strlen("!IF  \"$(CFG)\" == \""));
      } else {
        line.erase(0, strlen("!ELSEIF  \"$(CFG)\" == \""));
      }
      TargetConfig newConfig;
      newConfig.description = line.substr(0, line.length() - 1);
      data.configs.push_back(newConfig);
      activeConfig = &*(data.configs.end() - 1);
    } else if (line.find("# PROP Use_MFC ") != std::string::npos) {
      line.erase(0, strlen("# PROP Use_MFC "));
      data.useMFC = std::stoi(line);
    } else if (line.find("# ADD CPP ") != std::string::npos) {
      auto startCh = line.find("/D \"");

      while (startCh != std::string::npos) {
        line.erase(0, startCh + strlen("/D \""));

        activeConfig->definitions.emplace_back(line.substr(0, line.find('"')));

        startCh = line.find("/D \"");
      }
    } else if (line.find("# ADD LINK32 ") != std::string::npos) {
      line.erase(0, strlen("# ADD LINK32 "));
      // What we do on this line is try to extract any external libraries, the
      // output name, and use this to determine if the ultimate output is a
      // library or an executable.
      while (!line.empty()) {
        auto nextEnd = line.find(' ');
        std::string curSelection;
        if (nextEnd == std::string::npos) {
          nextEnd = line.size();
          curSelection = line.substr(0, nextEnd);
        } else {
          curSelection = line.substr(0, nextEnd++);
        }
        if (curSelection.find("/out:") != std::string::npos) {
          // Output
          curSelection.erase(0, strlen("/out:"));
          curSelection.erase(
              std::remove(curSelection.begin(), curSelection.end(), '\"'),
              curSelection.end());
          auto lastSlash = std::min(curSelection.find_last_of('/'),
                                    curSelection.find_last_of('\\'));
          if (lastSlash != std::string::npos) {
            lastSlash++;
            curSelection.erase(0, lastSlash);
          }
          data.isLibrary = checkIsLibrary(curSelection);
          activeConfig->name =
              curSelection.substr(0, curSelection.find_last_of('.'));
        } else if (curSelection.find(".lib") != std::string::npos ||
                   curSelection.find(".LIB") != std::string::npos ||
                   curSelection.find(".dll") != std::string::npos ||
                   curSelection.find(".DLL") != std::string::npos) {
          if (curSelection.find("/nodefaultlib:") != std::string::npos) {
            line.erase(0, strlen("/nodefaultlib:"));
            if (line[0] == '\"') {
              line.erase(0, line.find('\"', 1));
            }
            nextEnd = 0;
          } else {
            activeConfig->linkLibraries.emplace_back(curSelection);
          }
        }

        line.erase(0, nextEnd);
      }
    }
  }

  return std::make_tuple(true, data);
}