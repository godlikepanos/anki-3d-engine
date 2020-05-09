// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/shader_compiler/ShaderProgramBinary.h>
#include <anki/util/String.h>

namespace anki
{

/// @addtogroup shader_compiler
/// @{

/// Create a human readable representation of the shader binary.
void dumpShaderProgramBinary(const ShaderProgramBinary& binary, StringAuto& humanReadable);
/// @}

} // end namespace anki
