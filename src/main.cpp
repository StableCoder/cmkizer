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

// cmkizer
#include "file_parser.hpp"
#include "generators.hpp"
#include "util.hpp"

// C++
#include <string>

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
    generateCMakeTarget(targetData, true);
  }

  return 0;
}