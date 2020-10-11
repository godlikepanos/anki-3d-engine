// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shader_compiler/Common.h>
#include <anki/util/String.h>
#include <anki/gr/Enums.h>
#include <anki/gr/utils/Functions.h>

namespace anki
{

/// @addtogroup shader_compiler
/// @{

/// Run glslang's preprocessor.
ANKI_USE_RESULT Error preprocessGlsl(CString in, StringAuto& out);

/// Compile glsl to SPIR-V.
ANKI_USE_RESULT Error compilerGlslToSpirv(CString src, ShaderType shaderType, GenericMemoryPoolAllocator<U8> tmpAlloc,
										  DynamicArrayAuto<U8>& spirv);
/// @}

} // end namespace anki
