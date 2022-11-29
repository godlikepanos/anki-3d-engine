// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

enum class MaliOfflineCompilerHwUnit : U8
{
	kNone,
	kFma,
	kCvt,
	kSfu,
	kLoadStore,
	kVarying,
	kTexture
};

/// Mali offline compiler output.
class MaliOfflineCompilerOut
{
public:
	// Instructions
	F32 m_fma = -1.0f;
	F32 m_cvt = -1.0f;
	F32 m_sfu = -1.0f;
	F32 m_loadStore = -1.0f;
	F32 m_varying = -1.0f;
	F32 m_texture = -1.0f;
	MaliOfflineCompilerHwUnit m_boundUnit = MaliOfflineCompilerHwUnit::kNone;

	U32 m_workRegisters = kMaxU32;
	U32 m_spilling = 0;
	F32 m_fp16ArithmeticPercentage = 0.0f;

	void toString(StringRaii& str) const;
};

/// Run the mali offline compiler and get some info back.
Error runMaliOfflineCompiler(CString maliocExecutable, ConstWeakArray<U8> spirv, ShaderType shaderType,
							 BaseMemoryPool& tmpPool, MaliOfflineCompilerOut& out);
/// @}

} // end namespace anki
