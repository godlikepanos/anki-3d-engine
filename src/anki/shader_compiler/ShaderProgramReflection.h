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
	virtual ANKI_USE_RESULT Error setWorkgroupSizes(U32 x, U32 y, U32 z, U32 specConstMask) = 0;

	virtual ANKI_USE_RESULT Error setCounts(U32 uniformBlockCount, U32 storageBlockCount, U32 opaqueCount,
											Bool pushConstantBlock, U32 constsCount) = 0;

	virtual ANKI_USE_RESULT Error visitUniformBlock(U32 idx, CString name, U32 set, U32 binding, U32 size,
													U32 varCount) = 0;

	virtual ANKI_USE_RESULT Error visitUniformVariable(U32 blockIdx, U32 idx, CString name, ShaderVariableDataType type,
													   const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual ANKI_USE_RESULT Error visitStorageBlock(U32 idx, CString name, U32 set, U32 binding, U32 size,
													U32 varCount) = 0;

	virtual ANKI_USE_RESULT Error visitStorageVariable(U32 blockIdx, U32 idx, CString name, ShaderVariableDataType type,
													   const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual ANKI_USE_RESULT Error visitPushConstantsBlock(CString name, U32 size, U32 varCount) = 0;

	virtual ANKI_USE_RESULT Error visitPushConstant(U32 idx, CString name, ShaderVariableDataType type,
													const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual ANKI_USE_RESULT Error visitOpaque(U32 idx, CString name, ShaderVariableDataType type, U32 set, U32 binding,
											  U32 arraySize) = 0;

	virtual ANKI_USE_RESULT Error visitConstant(U32 idx, CString name, ShaderVariableDataType type, U32 constantId) = 0;
};

/// Does reflection using SPIR-V.
ANKI_USE_RESULT Error performSpirvReflection(Array<ConstWeakArray<U8>, U32(ShaderType::COUNT)> spirv,
											 GenericMemoryPoolAllocator<U8> tmpAlloc,
											 ShaderReflectionVisitorInterface& interface);
/// @}

} // end namespace anki
