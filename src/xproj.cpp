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

#include "xproj.hpp"

#include <libxml/parser.h>

#include <algorithm>
#include <filesystem>
#include <map>
#include <string>

#include "util.hpp"

void parseFilter(xmlNode *itemNode, TargetData &data) noexcept {

    std::string_view includeProp = (const char *)xmlGetProp(itemNode, (const xmlChar *)"Include");

    for (xmlNode *filterNode = itemNode->children; filterNode != nullptr;
         filterNode = filterNode->next) {
        std::string_view nodeName = (char const *)filterNode->name;

        if (nodeName == "Filter") {
            std::string filterName = (char const *)filterNode->children->content;

            for (auto it = filterName.begin(); it != filterName.end(); ++it) {
                if (*it == '\\') {
                    it = filterName.insert(it, '\\');
                    ++it;
                }
            }

            determineLanguage({includeProp.begin(), includeProp.end()}, data,
                              data.filters[filterName]);
        }
    }
}

void parseFiltersFile(std::string_view path, TargetData &data) noexcept {
    std::string filtersFilePath = path.data();
    filtersFilePath += ".filters";

    if (!std::filesystem::exists(filtersFilePath))
        return;

    xmlDoc *document = xmlReadFile(filtersFilePath.data(), nullptr, 0);
    if (document == nullptr)
        return;

    xmlNode *rootNode = xmlDocGetRootElement(document);
    if (rootNode == nullptr) {
        return;
    }

    for (xmlNode *groupNode = rootNode->children; groupNode != nullptr;
         groupNode = groupNode->next) {
        std::string_view nodeName = (char const *)groupNode->name;

        if (nodeName == "ItemGroup") {
            for (xmlNode *filterNode = groupNode->children; filterNode != nullptr;
                 filterNode = filterNode->next) {
                std::string_view nodeName = (char const *)filterNode->name;

                if (nodeName == "ClCompile" || nodeName == "ClInclude" ||
                    nodeName == "ResourceCompile") {
                    parseFilter(filterNode, data);
                }
            }
        }
    }
}

void parseProjectConfigurations(xmlNode *node, TargetData &data) noexcept {
    for (xmlNode *childNode = node->children; childNode != nullptr; childNode = childNode->next) {
        std::string_view nodeName = (char const *)childNode->name;

        if (nodeName == "ProjectConfiguration") {
            std::string includeProp =
                (char const *)xmlGetProp(childNode, (const xmlChar *)"Include");

            data.configs[includeProp];
        }
    }
}

void parseClCompile(xmlNode *node, TargetConfig &config) noexcept {
    for (xmlNode *childNode = node->children; childNode != nullptr; childNode = childNode->next) {
        std::string_view childName = (char const *)childNode->name;

        if (childName == "PreprocessorDefinitions") {
            auto defs = parseDefinitions((char const *)childNode->children->content);

            if (!defs.empty()) {
                config.definitions = defs;
            }
        }
    }
}

void parseLink(xmlNode *node, TargetConfig &config) noexcept {
    for (xmlNode *childNode = node->children; childNode != nullptr; childNode = childNode->next) {
        std::string_view childName = (char const *)childNode->name;

        if (childName == "AdditionalDependencies") {
            auto defs = parseDefinitions((char const *)childNode->children->content);

            if (!defs.empty()) {
                config.linkLibraries = defs;
            }
        } else if (childName == "AdditionalLibraryDirectories") {
            auto defs = parseDefinitions((char const *)childNode->children->content);

            if (!defs.empty()) {
                config.linkDirs = defs;
            }
        }
    }
}

void parseItemDefinitionGroup(xmlNode *node, TargetData &data) noexcept {
    std::string conditionProp = (char const *)xmlGetProp(node, (const xmlChar *)"Condition");

    for (auto &[name, config] : data.configs) {
        if (conditionProp.find(name) != std::string::npos) {
            for (xmlNode *childNode = node->children; childNode != nullptr;
                 childNode = childNode->next) {
                std::string_view childName = (char const *)childNode->name;

                if (childName == "ClCompile") {
                    parseClCompile(childNode, config);
                } else if (childName == "Link") {
                    parseLink(childNode, config);
                }
            }
        }
    }
}

std::tuple<bool, TargetData> xprojTargetParse(std::string_view targetPath) {
    TargetData data;
    data.fullPath = targetPath;

    parseFiltersFile(targetPath, data);

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

    for (xmlNode *rootChild = rootNode->children; rootChild != rootNode->last;
         rootChild = rootChild->next) {
        std::string_view childName = (const char *)rootChild->name;

        if (childName == "ItemGroup") {
            std::string_view itemGroupLabel =
                (const char *)xmlGetProp(rootChild, (const xmlChar *)"Label");

            if (itemGroupLabel == "ProjectConfigurations") {
                parseProjectConfigurations(rootChild, data);

            } else {
                // File/Project Reference Group

                for (xmlNode *fileNode = rootChild->children; fileNode != rootChild->last;
                     fileNode = fileNode->next) {
                    std::string_view nodeName = (const char *)fileNode->name;

                    if (nodeName == "ProjectReference") {
                        // It's a project dependency
                        for (xmlNode *dependencyNode = fileNode->children;
                             dependencyNode != fileNode->last;
                             dependencyNode = dependencyNode->next) {
                            std::string_view nodeName = (const char *)dependencyNode->name;

                            if (nodeName == "Project") {
                                data.dependencies.emplace_back(
                                    (const char *)dependencyNode->children->content);
                            }
                        }
                    } else {
                        // It's a source/header file
                        std::string_view includeName =
                            (const char *)xmlGetProp(fileNode, (const xmlChar *)"Include");

                        if (!includeName.empty())
                            data.allFiles.emplace_back(includeName);
                    }
                }
            }

        } else if (childName == "PropertyGroup") {
            std::string_view temp = (const char *)xmlGetProp(rootChild, (const xmlChar *)"Label");
            // Property Group - Globals
            if (temp == "Globals") {
                for (xmlNode *propGroup = rootNode->children; propGroup != rootNode->last;
                     propGroup = propGroup->next) {
                    std::string_view propGroupName = (const char *)propGroup->name;
                    if (propGroupName == "RootNamespace") {
                        data.name = (const char *)propGroup->content;
                    }
                }
            } else if (temp == "Configuration") {
                std::string_view condition =
                    (const char *)xmlGetProp(rootChild, (const xmlChar *)"Condition");
                for (auto &[name, config] : data.configs) {
                    if (condition.find(name) != std::string::npos) {
                        for (xmlNode *configNode = rootChild->children;
                             configNode != rootChild->last; configNode = configNode->next) {
                            std::string_view nodeName = (const char *)configNode->name;

                            if (nodeName == "ConfigurationType") {
                                std::string_view content =
                                    (const char *)configNode->children->content;
                                if (content.find("Library") != std::string::npos) {
                                    data.isLibrary = true;
                                }
                            }
                        }
                    }
                }
            }
        } else if (childName == "ItemDefinitionGroup") {
            parseItemDefinitionGroup(rootChild, data);
        }
    }

    return std::make_tuple(true, data);
}