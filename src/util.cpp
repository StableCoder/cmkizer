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

#include "util.hpp"

// C++
#include <algorithm>
#include <cstring>

void determineLanguage(const std::string_view fileName, TargetData &data,
                       FilterGroup &group) {
  std::string_view extension = fileName.data() + fileName.find_last_of('.') + 1;

  if (extension == "cpp" || extension == "CPP") {
    data.enableCXX = true;
    group.sourceFiles.emplace_back(fileName);
  } else if (extension == "cxx" || extension == "CXX") {
    data.enableC = true;
    group.sourceFiles.emplace_back(fileName);
  } else if (extension == "c" || extension == "C") {
    data.enableC = true;
    group.sourceFiles.emplace_back(fileName);
  } else if (extension == "for" || extension == "FOR") {
    data.enableFortran = true;
    group.sourceFiles.emplace_back(fileName);
  } else if (extension == "hpp" || extension == "h") {
    data.enableCXX = true;
    group.headerFiles.emplace_back(fileName);
  } else if (extension == "hxx" || extension == "HXX") {
    data.enableCXX = true;
    group.headerFiles.emplace_back(fileName);
  } else if (extension == "h" || extension == "H") {
    group.headerFiles.emplace_back(fileName);
  } else if (extension == "fi" || extension == "FI") {
    data.enableFortran = true;
    group.headerFiles.emplace_back(fileName);
  }
}

bool checkIsLibrary(const std::string_view outputName) {
  const char *extension = outputName.data() + outputName.find_last_of('.');
  return (strcmp(extension, ".lib") == 0 || strcmp(extension, ".LIB") == 0 ||
          strcmp(extension, ".dll") == 0 || strcmp(extension, ".DLL") == 0);
}

std::vector<std::string> parseItems(const std::string_view items) {
  std::vector<std::string> retList;

  std::size_t start = 0;
  std::size_t end =
      std::min(items.size(), std::min(items.find(';'), items.find(',')));

  while (start != items.size()) {
    std::string_view item(items.data() + start, end - start);
    if (item.find("%") == std::string::npos) {
      retList.emplace_back(item);
    }
    start = end + 1;
    if (start >= items.size()) {
      break;
    }
    end = std::min(items.size(),
                   std::min(items.find(';', start), items.find(',', start)));
  }

  return retList;
}

void removeDefaultDefinitions(std::vector<std::string> &definitionList) {
  for (auto it = definitionList.begin(); it != definitionList.end();) {
    if (*it == "WIN32" || *it == "_DEBUG" || *it == "NDEBUG" ||
        *it == "_WINDOWS" || *it == "_USRDLL" || *it == "__linux__") {
      definitionList.erase(it);
    } else {
      ++it;
    }
  }
}

void removeDefaultIncludes(std::vector<std::string> &includeList) {
  for (auto it = includeList.begin(); it != includeList.end();) {
    if (it->find("$(ConfigurationName") != std::string::npos) {
      includeList.erase(it);
    } else {
      ++it;
    }
  }
}