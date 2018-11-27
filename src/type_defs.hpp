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

#ifndef TYPE_DEFS_HPP
#define TYPE_DEFS_HPP

#include <map>
#include <string>
#include <vector>

/// A grouping of files together, separated as source, header and resource
/// files.
struct FilterGroup {
    /// List of files part of this filter group
    std::vector<std::string> files;
    /// True if there are sources to be compiled within this group
    bool sources{false};
    /// True if there are objects to be linked within this group
    bool objects{false};
};

/// A particular configuration of a target, including lists for the particular
/// includes, libraries and definitions.
struct TargetConfig {
    /// List of definitions
    std::vector<std::string> definitions;
    /// List of include directories
    std::vector<std::string> includeDirs;
    /// List of libraries to be linked
    std::vector<std::string> linkLibraries;
    /// List of directores containing libraries
    std::vector<std::string> linkDirs;
};

/// A full target's information, including the file's location, the configs,
/// files, and languages used.
struct TargetData {
    std::string name;
    std::string displayName;
    std::string fullPath;
    std::string relativePath;
    /// All the files of the target
    std::vector<std::string> allFiles;
    std::map<std::string, TargetConfig> configs;
    std::map<std::string, FilterGroup> filters;
    std::vector<std::string> dependencies;
    bool enableC = false;
    bool enableCXX = false;
    bool enableFortran = false;
    bool isLibrary = false;
    int useMFC = 0;
    bool useQt = false;
};

/// A full project's information.
struct ProjectData {
    /// Name of the whole project
    std::string name;
    /// The path as to where the project is located
    std::string path;
    /// The set of targets within the project
    std::vector<TargetData> targets;
};

struct GlobalSettings {
    int qtVersion = 0;
    std::string includePath = "include/";
    std::string cmakeVersion = "3.13";
    int cpackType = 0;
};

constexpr const char *cCmakeFilename("CMakeLists.txt");

#endif // TYPE_DEFS_HPP
