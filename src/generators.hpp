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

#ifndef GENERATORS_HPP
#define GENERATORS_HPP

// cmkizer
#include "type_defs.hpp"

// C++
#include <string>

/// Preprocesses a project's data to fill in any gaps/holes in data as best as
/// can be guessed, aswell as links up dependencies between targets.
/// \param data The ProjectData to process.
/// \return A processed ProjectData struct.
ProjectData projectPreprocessing(ProjectData data);

/// Generates a CMake file using the provided project data
/// \param projectData The data to use to construct the CMake file.
void generateCMakeProject(const ProjectData &projectData);

/// Generates a CMake file using the provided target data.
/// \param data The TargetData to use.
/// \param standalone If true, then the target is written as if a standalone
/// project/target combination.
/// \param pOutfile If specified, then this will be file the output is written
/// to.
void generateCMakeTarget(const TargetData &data, bool standalone = false, FILE *pOutfile = nullptr);

#endif // GENERATORS_HPP
