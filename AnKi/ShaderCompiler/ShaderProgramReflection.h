// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Gr/Common.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

/// A visitor class that will be used to populate reflection information.
class ShaderReflectionVisitorInterface
{
public:
	virtual Error setWorkgroupSizes(U32 x, U32 y, U32 z, U32 specConstMask) = 0;

	virtual Error setCounts(U32 uniformBlockCount, U32 storageBlockCount, U32 opaqueCount, Bool pushConstantBlock,
							U32 constsCount, U32 structCount) = 0;

	virtual Error visitUniformBlock(U32 idx, CString name, U32 set, U32 binding, U32 size, U32 varCount) = 0;

	virtual Error visitUniformVariable(U32 blockIdx, U32 idx, CString name, ShaderVariableDataType type,
									   const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual Error visitStorageBlock(U32 idx, CString name, U32 set, U32 binding, U32 size, U32 varCount) = 0;

	virtual Error visitStorageVariable(U32 blockIdx, U32 idx, CString name, ShaderVariableDataType type,
									   const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual Error visitPushConstantsBlock(CString name, U32 size, U32 varCount) = 0;

	virtual Error visitPushConstant(U32 idx, CString name, ShaderVariableDataType type,
									const ShaderVariableBlockInfo& blockInfo) = 0;

	virtual Error visitOpaque(U32 idx, CString name, ShaderVariableDataType type, U32 set, U32 binding,
							  U32 arraySize) = 0;

	virtual Error visitConstant(U32 idx, CString name, ShaderVariableDataType type, U32 constantId) = 0;

	virtual Error visitStruct(U32 idx, CString name, U32 memberCount, U32 size) = 0;

	virtual Error visitStructMember(U32 structIdx, CString structName, U32 memberIdx, CString memberName,
									ShaderVariableDataType type, CString typeStructName, U32 offset, U32 arraySize) = 0;

	[[nodiscard]] virtual Bool skipSymbol(CString symbol) const = 0;
};

/// Does reflection using SPIR-V.
Error performSpirvReflection(Array<ConstWeakArray<U8>, U32(ShaderType::kCount)> spirv, BaseMemoryPool& tmpPool,
							 ShaderReflectionVisitorInterface& interface);
/// @}

} // end namespace anki
