// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shader_compiler/Common.h>
#include <anki/gr/Enums.h>
#include <anki/util/WeakArray.h>

namespace anki
{

// Forward
class ShaderProgramBinaryReflection;

/// @addtogroup shader_compiler
/// @{

ANKI_USE_RESULT Error performSpirvReflection(ShaderProgramBinaryReflection& refl,
	Array<ConstWeakArray<U8, PtrSize>, U32(ShaderType::COUNT)> spirv,
	GenericMemoryPoolAllocator<U8> tmpAlloc,
	GenericMemoryPoolAllocator<U8> binaryAlloc);
/// @}

} // end namespace anki
