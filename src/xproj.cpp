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

#include "xproj.hpp"

// cmkizer
#include "util.hpp"

// libxml2
#include <libxml/parser.h>

// C++
#include <algorithm>

std::tuple<bool, TargetData>
xprojTargetParse(const std::string_view targetPath) {
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

  auto start =
      std::min(targetPath.find_last_of('\\'), targetPath.find_last_of('/'));
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

        for (xmlNode *projConfig = rootChild->children;
             projConfig != rootChild->last; projConfig = projConfig->next) {
          std::string_view nodeName = (const char *)projConfig->name;

          if (nodeName == "ProjectConfiguration") {
            TargetConfig config;
            config.name = data.name;
            config.description = (const char *)xmlGetProp(
                projConfig, (const xmlChar *)"Include");
            data.configs.emplace_back(std::move(config));
          }
        }

      } else {
        // File Group
        FilterGroup group;

        for (xmlNode *fileNode = rootChild->children;
             fileNode != rootChild->last; fileNode = fileNode->next) {
          std::string_view includeName =
              (const char *)xmlGetProp(fileNode, (const xmlChar *)"Include");

          if (!includeName.empty()) {
            determineLanguage(includeName, data, group);
          }
        }

        data.groups.emplace_back(std::move(group));
      }
    } else if (childName == "PropertyGroup") {
      std::string_view temp =
          (const char *)xmlGetProp(rootChild, (const xmlChar *)"Label");
      // Property Group - Globals
      if (temp == "Globals") {
        for (xmlNode *propGroup = rootNode->children;
             propGroup != rootNode->last; propGroup = propGroup->next) {
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