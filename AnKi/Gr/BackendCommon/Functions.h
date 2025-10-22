// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/BackendCommon/Common.h>
#include <AnKi/Math.h>

namespace anki {

inline Bool stencilTestEnabled(StencilOperation stencilFail, StencilOperation stencilPassDepthFail, StencilOperation stencilPassDepthPass,
							   CompareOperation compare)
{
	return stencilFail != StencilOperation::kKeep || stencilPassDepthFail != StencilOperation::kKeep
		   || stencilPassDepthPass != StencilOperation::kKeep || compare != CompareOperation::kAlways;
}

inline Bool depthTestEnabled(CompareOperation depthCompare, Bool depthWrite)
{
	return depthCompare != CompareOperation::kAlways || depthWrite;
}

inline Bool blendingEnabled(BlendFactor srcFactorRgb, BlendFactor dstFactorRgb, BlendFactor srcFactorA, BlendFactor dstFactorA, BlendOperation opRgb,
							BlendOperation opA)
{
	const Bool dontWantBlend = srcFactorRgb == BlendFactor::kOne && dstFactorRgb == BlendFactor::kZero && srcFactorA == BlendFactor::kOne
							   && dstFactorA == BlendFactor::kZero && (opRgb == BlendOperation::kAdd || opRgb == BlendOperation::kSubtract)
							   && (opA == BlendOperation::kAdd || opA == BlendOperation::kSubtract);
	return !dontWantBlend;
}

/// Using an AnKi typename get the ShaderVariableDataType. Used for debugging.
template<typename T>
ShaderVariableDataType getShaderVariableTypeFromTypename();

#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	template<> \
	inline ShaderVariableDataType getShaderVariableTypeFromTypename<type>() \
	{ \
		return ShaderVariableDataType::k##type; \
	}
#include <AnKi/Gr/ShaderVariableDataType.def.h>

/// Populate the memory of a variable that is inside a shader block.
void writeShaderBlockMemory(ShaderVariableDataType type, const ShaderVariableBlockInfo& varBlkInfo, const void* elements, U32 elementsCount,
							void* buffBegin, const void* buffEnd);

} // end namespace anki
