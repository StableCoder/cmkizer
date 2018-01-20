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

// cmkizer
#include "file_parser.hpp"
#include "generators.hpp"
#include "util.hpp"

// C++
#include <string>
#include <string_view>

void printHelp() {
    printf("Usage: cmkizer [OPTION] [FILE]\n"
           "A utility that guestimates from a visual studio solution a fairly "
           "close\n"
           "basic approximation of an equivalent cmake project and files.\n"
           "Example: cmkizer -i \"MSVS Sol.sln\"\n"
           "\n"
           "  -qt <int>   specifies a Qt version to setup the project for\n"
           "  -v <str>    changes the string of the CMake version used(default "
           "'3.9')\n"
           "  -i <str>    changes the include path for installing "
           "headers(default 'include/')\n"
           "  --version   print version number\n"
           "  --help      show this help\n");
}

void printVersion() { printf("cmkizer 17.0"); }

int main(int argc, char **argv) {
    if (argc == 1) {
        printHelp();
        return 0;
    }

    // Process the command line arguments, if any.
    for (int idx = 1; idx < argc; ++idx) {
        std::string_view arg = argv[idx];

        if (arg == "--help") {
            printHelp();
            return 0;
        }
        if (arg == "--version") {
            printVersion();
            return 0;
        }
        if (arg == "-qt" && idx < argc) {
            gGlobalSettings.qtVersion = std::stoi(argv[idx + 1]);
        }
        if (arg == "-v" && idx < argc) {
            gGlobalSettings.cmakeVersion = argv[idx + 1];
        }
        if (arg == "-i" && idx < argc) {
            gGlobalSettings.includePath = argv[idx + 1];
        }
        if (arg == "-p") {
            gGlobalSettings.cpackType = 1;
        }
        if (arg == "-d") {
            gGlobalSettings.cpackType = 2;
        }
    }

    // The last one should be the file we're operating upon, attempt to open it.
    auto[projSuccess, projData] = parseProject(argv[argc - 1]);

    if (projSuccess) {
        projData = projectPreprocessing(projData);
        generateCMakeProject(projData);
        return 0;
    }

    auto[targetSuccess, targetData] = parseTarget(argv[argc - 1]);

    if (targetSuccess) {
        ProjectData temp;
        temp.targets.emplace_back(targetData);
        temp = projectPreprocessing(temp);
        generateCMakeTarget(temp.targets[0]);
    }

    return 0;
}