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

#include "vfproj.hpp"

#include <libxml/parser.h>

#include <algorithm>

#include "util.hpp"

void parseToolNode(xmlNode const *toolNode, TargetConfig &data) noexcept {
    std::string_view nodeName = (const char *)toolNode->name;

    if (nodeName == "Tool") {
        std::string_view toolName = (const char *)xmlGetProp(toolNode, (const xmlChar *)"Name");

        if (toolName == "VFFortranCompilerTool") {
            std::string_view includeDir =
                (const char *)xmlGetProp(toolNode, (const xmlChar *)"AdditionalIncludeDirectories");

            if (!includeDir.empty())
                data.includeDirs.emplace_back(includeDir);
        } else if (toolName == "VFLinkerTool") {
            std::string_view linkDir =
                (const char *)xmlGetProp(toolNode, (const xmlChar *)"AdditionalLibraryDirectories");

            if (!linkDir.empty())
                data.linkDirs.emplace_back(linkDir);
        }
    }
}

void parseConfigurationNode(xmlNode const *configNode, TargetData &data) noexcept {
    std::string_view nodeName = (const char *)configNode->name;

    if (nodeName == "Configuration") {
        std::string_view confName = (const char *)xmlGetProp(configNode, (const xmlChar *)"Name");

        for (xmlNode *toolNode = configNode->children; toolNode != nullptr;
             toolNode = toolNode->next) {
            parseToolNode(toolNode, data.configs[{confName.begin(), confName.end()}]);
        }
    }
}

void parseFilterNode(xmlNode const *filterNode, TargetData &data) noexcept {
    std::string_view nodeName = (const char *)filterNode->name;

    if (nodeName == "Filter") {
        std::string_view filterName = (const char *)xmlGetProp(filterNode, (const xmlChar *)"Name");

        FilterGroup filter;

        for (xmlNode *fileNode = filterNode->children; fileNode != nullptr;
             fileNode = fileNode->next) {
            std::string_view nodeName = (const char *)fileNode->name;

            if (nodeName == "File") {
                std::string_view path =
                    (const char *)xmlGetProp(fileNode, (const xmlChar *)"RelativePath");

                data.allFiles.emplace_back(path);
                determineLanguage({path.begin(), path.end()}, data, filter);
            }
        }

        data.filters[{filterName.begin(), filterName.end()}] = filter;
    }
}

std::tuple<bool, TargetData> vfprojTargetParse(std::string_view targetPath) noexcept {
    TargetData data;
    data.enableFortran = true;
    data.fullPath = targetPath;

    xmlDoc *document = xmlReadFile(targetPath.data(), nullptr, 0);
    if (document == nullptr) {
        return std::make_tuple(false, data);
    }
    xmlNode *rootNode = xmlDocGetRootElement(document);
    if (rootNode == nullptr) {
        return std::make_tuple(false, data);
    }

    auto start = std::min(targetPath.find_last_of('\\'), targetPath.find_last_of('/'));
    if (start == std::string::npos) {
        start = 0;
    } else {
        ++start;
    }
    auto end = targetPath.find_last_of('.');
    data.name = targetPath.substr(start, end - start);

    for (xmlNode *rootChild = rootNode->children; rootChild != nullptr;
         rootChild = rootChild->next) {
        std::string_view childName = (const char *)rootChild->name;
        std::string_view type = (const char *)xmlGetProp(rootNode, (const xmlChar *)"ProjectType");

        if (type.find("Library") != std::string::npos) {
            data.isLibrary = true;
        }

        if (childName == "Configurations") {
            // Configurations Node

            for (xmlNode *node = rootChild->children; node != nullptr; node = node->next) {
                parseConfigurationNode(node, data);
            }

        } else if (childName == "Files") {
            // Filter/Files Node

            for (xmlNode *filterNode = rootChild->children; filterNode != nullptr;
                 filterNode = filterNode->next) {
                parseFilterNode(filterNode, data);
            }
        }
    }

    return std::make_tuple(true, data);
}