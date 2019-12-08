// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
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

/// A variable inside a uniform or storage blocks or push constants.
class GlslReflectionBlockVariable
{
public:
	String m_name;
	ShaderVariableBlockInfo m_info;
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
};

/// A uniform buffer or storage buffer.
class GlslReflectionBlock
{
public:
	DynamicArray<GlslReflectionBlockVariable> m_variables;
	PtrSize m_size = MAX_PTR_SIZE;
	U32 m_binding = MAX_U32;
	U8 m_set = MAX_U8;
};

/// A spec constant variable.
class GlslReflectionSpecializationConstant
{
public:
	String m_name;
	U32 m_id = MAX_U32;
	ShaderVariableDataType m_type = ShaderVariableDataType::NONE;
};

/// GLSL reflection.
class GlslReflection
{
public:
	GlslReflection(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~GlslReflection();

	DynamicArray<GlslReflectionBlock> m_uniformBlocks;
	DynamicArray<GlslReflectionBlock> m_storageBlocks;
	GlslReflectionBlock* m_pushConstants = nullptr;
	DynamicArray<GlslReflectionSpecializationConstant> m_specializationConstants;

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
};

/// Run glslang's preprocessor.
ANKI_USE_RESULT Error preprocessGlsl(CString in, StringAuto& out);

/// Compile glsl to SPIR-V.
ANKI_USE_RESULT Error compilerGlslToSpirv(
	CString src, ShaderType shaderType, GenericMemoryPoolAllocator<U8> tmpAlloc, DynamicArrayAuto<U8>& spirv);
/// @}

} // end namespace anki
