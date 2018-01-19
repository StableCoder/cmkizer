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

#include "util.hpp"

// C++
#include <algorithm>
#include <cstring>

void determineLanguage(const std::string_view fileName, TargetData &data, FilterGroup &group) {
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
    std::size_t end = std::min(items.size(), std::min(items.find(';'), items.find(',')));

    while (start != items.size()) {
        std::string_view item(items.data() + start, end - start);
        if (item.find("%") == std::string::npos) {
            retList.emplace_back(item);
        }
        start = end + 1;
        if (start >= items.size()) {
            break;
        }
        end = std::min(items.size(), std::min(items.find(';', start), items.find(',', start)));
    }

    return retList;
}

void removeDefaultDefinitions(std::vector<std::string> &definitionList) {
    for (auto it = definitionList.begin(); it != definitionList.end();) {
        if (*it == "WIN32" || *it == "_DEBUG" || *it == "NDEBUG" || *it == "_WINDOWS" ||
            *it == "_USRDLL" || *it == "__linux__") {
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