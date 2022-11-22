// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/String.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

/// Compile HLSL to SPIR-V.
Error compileHlslToSpirv(CString src, ShaderType shaderType, BaseMemoryPool& tmpPool, DynamicArrayRaii<U8>& spirv,
						 StringRaii& errorMessage);
/// @}

} // end namespace anki
