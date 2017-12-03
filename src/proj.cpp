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