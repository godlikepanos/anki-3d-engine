// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

/// RGA output.
class RgaOutput
{
public:
	U32 m_vgprCount;
	U32 m_sgprCount;
	U32 m_isaSize;

	void toString(String& str) const;
};

/// Run the mali offline compiler and get some info back.
Error runRadeonGpuAnalyzer(CString rgaExecutable, ConstWeakArray<U8> spirv, ShaderType shaderType, RgaOutput& out);
/// @}

} // end namespace anki
