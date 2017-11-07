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
void generateCMakeTarget(const TargetData &data, bool standalone = false,
                         FILE *pOutfile = nullptr);

#endif // GENERATORS_HPP
