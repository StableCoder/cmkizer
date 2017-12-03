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

#include "proj.hpp"

// cmkizer
#include "util.hpp"

// libxml2
#include <libxml/parser.h>

// C++
#include <algorithm>
#include <cstring>
#include <cctype>
#include <fstream>

std::tuple<bool, TargetData>
projTargetParse(const std::string_view targetPath) {
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
  {
    const char *temp =
        (const char *)xmlGetProp(rootNode, (const xmlChar *)"RootNamespace");
    if (temp != nullptr) {
      data.name = temp;
    }
    temp = (const char *)xmlGetProp(rootNode, (const xmlChar *)"Name");
    if (temp != nullptr) {
      data.name = temp;
    }
  }

  // Root Children
  for (xmlNode *rootChild = rootNode->children; rootChild != rootNode->last;
       rootChild = rootChild->next) {
    std::string_view rootChildName = (const char *)rootChild->name;

    if (rootChildName == "Configurations") {
      for (xmlNode *configNode = rootChild->children;
           configNode != rootChild->last; configNode = configNode->next) {
        std::string_view configNodeName = (const char *)configNode->name;

        if (configNodeName == "Configuration") {
          TargetConfig config;
          config.name = data.name;
          config.description =
              (const char *)xmlGetProp(configNode, (const xmlChar *)"Name");
          auto temp = (const char *)xmlGetProp(
              configNode, (const xmlChar *)"ConfigurationType");
          if (temp != nullptr) {
            if (temp == "typeDynamicLibrary" || temp == "2" ||
                temp == "typeStaticLibrary") {
              data.isLibrary = true;
            }
          }

          for (xmlNode *toolNode = configNode->children;
               toolNode != configNode->last; toolNode = toolNode->next) {
            std::string_view toolNodeName = (const char *)toolNode->name;

            if (toolNodeName == "Tool") {
              std::string_view toolName =
                  (const char *)xmlGetProp(toolNode, (const xmlChar *)"Name");

              if (toolName == "VCCLCompilerTool" ||
                  toolName == "VFFortranCompilerTool") {
                auto temp = (const char *)xmlGetProp(
                    toolNode, (const xmlChar *)"PreprocessorDefinitions");
                if (temp != nullptr) {
                  config.definitions = parseItems(temp);
                }
                temp = (const char *)xmlGetProp(
                    toolNode, (const xmlChar *)"AdditionalIncludeDirectories");
                if (temp != nullptr) {
                  config.includes = parseItems(temp);
                }
              } else if (toolName == "VCLinkerTool" ||
                         toolName == "VFLinkerTool") {
                auto temp = (const char *)xmlGetProp(
                    toolNode, (const xmlChar *)"OutputFile");
                if (temp != nullptr) {
                  data.isLibrary = checkIsLibrary(temp);
                }
                temp = (const char *)xmlGetProp(
                    toolNode, (const xmlChar *)"AdditionalDependencies");
                if (temp != nullptr) {
                  config.linkLibraries = parseItems(temp);
                }
              }
            }
          }
          data.configs.emplace_back(std::move(config));
        }
      }
    } else if (rootChildName == "Files") {
      for (xmlNode *filterNode = rootChild->children;
           filterNode != rootChild->last; filterNode = filterNode->next) {
        std::string_view filterNodeName = (const char *)filterNode->name;

        if (filterNodeName == "Filter") { // Filter Properties
          FilterGroup group;
          group.name =
              (const char *)xmlGetProp(filterNode, (const xmlChar *)"Name");
          std::replace(group.name.begin(), group.name.end(), ' ', '_');
          std::transform(
              group.name.begin(), group.name.end(), group.name.begin(),
              [](unsigned char c) -> unsigned char { return std::toupper(c); });

          for (xmlNode *fileNode = filterNode->children;
               fileNode != filterNode->last; fileNode = fileNode->next) {
            std::string_view fileNodeName = (const char *)fileNode->name;
            if (fileNodeName == "File") {
              // File properties
              std::string fileName = (const char *)xmlGetProp(
                  fileNode, (const xmlChar *)"RelativePath");
              std::replace(fileName.begin(), fileName.end(), '\\', '/');
              if (fileName[0] == '.' && fileName[1] == '/') {
                fileName.erase(0, 2);
              }
              determineLanguage(fileName, data, group);
            }
          }
          data.groups.emplace_back(std::move(group));
        }
      }
    }
  }

  return std::make_tuple(true, data);
}