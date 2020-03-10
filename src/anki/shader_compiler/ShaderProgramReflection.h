// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shader_compiler/Common.h>
#include <anki/gr/Enums.h>
#include <anki/gr/Common.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup shader_compiler
/// @{

/// A visitor class that will be used to populate reflection information.
class ShaderReflectionVisitorInterface
{
public:
	virtual void setUniformBlockCount(U32 count) = 0;

	virtual void visitUniformBlock(CString name, U32 set, U32 binding, U32 size) = 0;

	virtual void setUniformBlockVariableCount(U32 count) = 0;

	virtual void visitUniformVariable(
		CString name, ShaderVariableDataType type, const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual void setStorageBlockCount(U32 count) = 0;

	virtual void visitStorageBlock(CString name, U32 set, U32 binding, U32 size) = 0;

	virtual void setStorageBlockVariableCount(U32 count) = 0;

	virtual void visitStorageVariable(
		CString name, ShaderVariableDataType type, const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual void visitPushConstantsBlock(CString name, U32 size, U32 varCount) = 0;

	virtual void visitPushConstant(
		CString name, ShaderVariableDataType type, const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual void setOpaqueCount(U32 count) = 0;

	virtual void visitOpaque(CString name, ShaderVariableDataType type, U32 set, U32 binding, U32 arraySize) = 0;

	virtual void setConstantCount(U32 count) = 0;

	virtual void visitConstant(CString name, ShaderVariableDataType type, U32 constantId, ShaderTypeBit stages) = 0;
};

/// Does reflection using SPIR-V.
ANKI_USE_RESULT Error performSpirvReflection(Array<ConstWeakArray<U8, PtrSize>, U32(ShaderType::COUNT)> spirv,
	GenericMemoryPoolAllocator<U8> tmpAlloc,
	ShaderReflectionVisitorInterface& interface);
/// @}

} // end namespace anki
