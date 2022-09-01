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

/// Run glslang's preprocessor.
Error preprocessGlsl(CString in, StringAuto& out);

/// Compile glsl to SPIR-V.
Error compilerGlslToSpirv(CString src, ShaderType shaderType, GenericMemoryPoolAllocator<U8> tmpAlloc,
						  DynamicArrayAuto<U8>& spirv, StringAuto& errorMessage);
/// @}

} // end namespace anki
