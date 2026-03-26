// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// RGA output.
class RgaOutput
{
public:
	U32 m_vgprCount = 0;
	U32 m_sgprCount = 0;
	U32 m_isaSize = 0;

	ShaderCompilerString m_isa;

	String toString() const
	{
		return String().sprintf("VGPRs %u SGPRs %u ISA size %u", m_vgprCount, m_sgprCount, m_isaSize);
	}
};

// Run the radeon GPU analyzer and get a few things back.
Error runRadeonGpuAnalyzer(ConstWeakArray<U8> spirv, ShaderType shaderType, RgaOutput& out);

} // end namespace anki
