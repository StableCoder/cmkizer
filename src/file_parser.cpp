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
#include "dsp.hpp"
#include "dsw.hpp"

// C++
#include <cstring>

std::tuple<bool, ProjectData> parseProject(const std::string &projectPath) {
  // Figure out the file type.
  const auto lastDot(projectPath.find_last_of('.'));
  if (lastDot != std::string::npos) {
    const char *ext = projectPath.data() + lastDot;

    if (strcmp(ext, ".dsw") == 0 || strcmp(ext, ".DSW") == 0) {
      return dswProjectParse(projectPath);
    }
  }

  // We didn't process anything
  return std::make_tuple(false, ProjectData());
}

std::tuple<bool, TargetData> parseTarget(const std::string &targetPath) {
  const auto lastDot = targetPath.find_last_of('.');
  if (lastDot != std::string::npos) {
    const char *ext = targetPath.data() + lastDot;

    if (strcmp(ext, ".dsp") == 0 || strcmp(ext, ".DSP") == 0) {
      return dspTargetParse(targetPath);
    }
  }

  return std::make_tuple(false, TargetData());
};