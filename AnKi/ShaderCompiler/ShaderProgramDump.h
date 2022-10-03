// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderProgramBinary.h>
#include <AnKi/Util/String.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

/// Create a human readable representation of the shader binary.
void dumpShaderProgramBinary(const ShaderProgramBinary& binary, StringRaii& humanReadable);
/// @}

} // end namespace anki
