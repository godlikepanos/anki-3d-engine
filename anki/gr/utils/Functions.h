// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/Math.h>

namespace anki
{

inline Bool stencilTestDisabled(StencilOperation stencilFail, StencilOperation stencilPassDepthFail,
								StencilOperation stencilPassDepthPass, CompareOperation compare)
{
	return stencilFail == StencilOperation::KEEP && stencilPassDepthFail == StencilOperation::KEEP
		   && stencilPassDepthPass == StencilOperation::KEEP && compare == CompareOperation::ALWAYS;
}

inline Bool blendingDisabled(BlendFactor srcFactorRgb, BlendFactor dstFactorRgb, BlendFactor srcFactorA,
							 BlendFactor dstFactorA, BlendOperation opRgb, BlendOperation opA)
{
	Bool dontWantBlend = srcFactorRgb == BlendFactor::ONE && dstFactorRgb == BlendFactor::ZERO
						 && srcFactorA == BlendFactor::ONE && dstFactorA == BlendFactor::ZERO
						 && (opRgb == BlendOperation::ADD || opRgb == BlendOperation::SUBTRACT)
						 && (opA == BlendOperation::ADD || opA == BlendOperation::SUBTRACT);
	return dontWantBlend;
}

/// Using an AnKi typename get the ShaderVariableDataType. Used for debugging.
template<typename T>
ShaderVariableDataType getShaderVariableTypeFromTypename();

#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount) \
	template<> \
	inline ShaderVariableDataType getShaderVariableTypeFromTypename<type>() \
	{ \
		return ShaderVariableDataType::capital; \
	}

#include <anki/gr/ShaderVariableDataTypeDefs.h>
#undef ANKI_SVDT_MACRO

/// Populate the memory of a variable that is inside a shader block.
void writeShaderBlockMemory(ShaderVariableDataType type, const ShaderVariableBlockInfo& varBlkInfo,
							const void* elements, U32 elementsCount, void* buffBegin, const void* buffEnd);

/// Convert a ShaderVariableDataType to string.
const CString shaderVariableDataTypeToString(ShaderVariableDataType t);

} // end namespace anki
