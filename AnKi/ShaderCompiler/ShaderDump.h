// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/ShaderBinary.h>
#include <AnKi/Util/String.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

class ShaderDumpOptions
{
public:
	Bool m_writeGlsl = true;
	Bool m_writeSpirv = false;
	Bool m_maliStats = false;
	Bool m_amdStats = false;
};

/// Create a human readable representation of the shader binary.
void dumpShaderBinary(const ShaderDumpOptions& options, const ShaderBinary& binary, ShaderCompilerString& humanReadable);
/// @}

} // end namespace anki
