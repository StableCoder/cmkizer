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
#include "dsp.hpp"
#include "dsw.hpp"
#include "proj.hpp"
#include "sln.hpp"
#include "xproj.hpp"

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
        if (strcmp(ext, ".sln") == 0 || strcmp(ext, ".SLN") == 0) {
            return slnProjectParse(projectPath);
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
        if (strcmp(ext, ".proj") == 0 || strcmp(ext, ".PROJ") == 0) {
            return projTargetParse(targetPath);
        }
        if (strcmp(ext, ".xproj") == 0 || strcmp(ext, ".XPROJ") == 0) {
            return xprojTargetParse(targetPath);
        }
    }

    return std::make_tuple(false, TargetData());
};