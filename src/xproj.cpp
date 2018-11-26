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

// cmkizer
#include "util.hpp"

// libxml2
#include <libxml/parser.h>

// C++
#include <algorithm>

std::tuple<bool, TargetData> xprojTargetParse(std::string_view targetPath) {
    TargetData data;
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

    // Root Properties
    {}

    for (xmlNode *rootChild = rootNode->children; rootChild != rootNode->last;
         rootChild = rootChild->next) {
        std::string_view childName = (const char *)rootChild->name;

        if (childName == "ItemGroup") {
            std::string_view itemGroupLabel =
                (const char *)xmlGetProp(rootChild, (const xmlChar *)"Label");

            if (itemGroupLabel == "ProjectConfigurations") {
                // Project Configurations

                for (xmlNode *projConfig = rootChild->children; projConfig != rootChild->last;
                     projConfig = projConfig->next) {
                    std::string_view nodeName = (const char *)projConfig->name;

                    if (nodeName == "ProjectConfiguration") {
                        TargetConfig config;
                        config.name = data.name;
                        config.description =
                            (const char *)xmlGetProp(projConfig, (const xmlChar *)"Include");
                        data.configs.emplace_back(std::move(config));
                    }
                }

            } else {
                // File Group
                FilterGroup group;

                for (xmlNode *fileNode = rootChild->children; fileNode != rootChild->last;
                     fileNode = fileNode->next) {
                    std::string_view includeName =
                        (const char *)xmlGetProp(fileNode, (const xmlChar *)"Include");

                    if (!includeName.empty()) {
                        determineLanguage({includeName.begin(), includeName.end()}, data, group);
                    }
                }

                data.groups.emplace_back(std::move(group));
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
                for (auto &config : data.configs) {
                    if (condition.find(config.description) != std::string::npos) {
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
        }
    }

    return std::make_tuple(true, data);
}