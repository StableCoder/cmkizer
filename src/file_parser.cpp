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

// cmkizer
#include "file_parser.hpp"
//#include "dsp.hpp"
//#include "dsw.hpp"
//#include "proj.hpp"
#include "sln.hpp"
#include "vfproj.hpp"
#include "xproj.hpp"

// C++
#include <algorithm>
#include <cctype>

std::tuple<bool, ProjectData> parseProject(std::string_view projectPath) {
    // Figure out the file type.
    const auto lastDot(projectPath.find_last_of('.'));
    if (lastDot != std::string::npos) {
        std::string ext = projectPath.data() + lastDot;

        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".dsw") {
            // return dswProjectParse(projectPath);
        }
        if (ext == ".sln") {
            return slnProjectParse(projectPath);
        }
    }

    // We didn't process anything
    return std::make_tuple(false, ProjectData());
}

std::tuple<bool, TargetData> parseTarget(std::string_view targetPath) {
    const auto lastDot = targetPath.find_last_of('.');
    if (lastDot != std::string::npos) {
        std::string ext = targetPath.data() + lastDot;

        if (ext == ".dsp") {
            // return dspTargetParse(targetPath);
        }
        if (ext == ".vcproj") {
            // return projTargetParse(targetPath);
        }
        if (ext == ".vcxproj") {
            return xprojTargetParse(targetPath);
        }
        if (ext == ".vfproj") {
            return vfprojTargetParse(targetPath);
        }
    }

    return std::make_tuple(false, TargetData());
};